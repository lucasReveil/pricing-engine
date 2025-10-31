#include "bootstrap.hpp"
#include "curve.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <vector>

namespace pricer {

inline std::vector<Pillar>
build_pillars_from_f(const std::vector<double> &f_seg,
                     const std::vector<double> &T, int frequency) {

  auto seg_length_up_to = [&](double t, std::size_t j) -> double {
    const double a = (j == 0) ? 0.0 : T[j - 1];
    const double b = T[j];
    const double x = std::min(std::max(t, 0.0), b);
    return std::max(0.0, x - a);
  };
  std::vector<Pillar> pillars;
  const double dt = 1.0 / static_cast<double>(frequency);
  const double t_max = T.back();
  const int steps = static_cast<int>(std::lround(t_max / dt));

  pillars.reserve(static_cast<std::size_t>(steps));
  for (int k = 1; k <= steps; ++k) {
    const double t = k * dt;
    double I = 0.0;
    for (std::size_t j = 0; j < f_seg.size(); ++j) {
      const double len = seg_length_up_to(t, j);
      if (len > 0.0) {
        I += f_seg[j] * len;
      }
      if (t <= T[j]) {
        break;
      }
    }
    const double df = std::exp(-I);
    pillars.push_back(Pillar{t, df});
  }
  return pillars;
}

BootstrapResult boostrap_piecewise_forward(const std::vector<Bond> &bonds,
                                           const std::vector<double> &prices,
                                           double f_lo, double f_hi, double tol,
                                           int maxit) {
  if (bonds.empty() || bonds.size() != prices.size()) {
    throw std::invalid_argument(
        "bootstrap: bonds/prices size mismatch or empty");
  }
  std::vector<Bond> bs = bonds;
  std::sort(bs.begin(), bs.end(), [](const Bond &a, const Bond &b) {
    return a.maturity() < b.maturity();
  });
  const int freq = bs.front().frequency();
  for (const auto &b : bs) {
    if (b.frequency() != freq) {
      throw std::invalid_argument(
          "bootstrap: all bonds must share the same frequency");
    }
  }
  std::vector<double> T;
  T.reserve(bs.size());
  for (const auto &b : bs) {
    T.push_back((b.maturity()));
  }
  // local helpers
  auto logD_known = [&](double t, const std::vector<double> &f,
                        std::size_t known_k) -> double {
    double I = 0.0;
    for (std::size_t j = 0; j < known_k; ++j) {
      const double a = (j == 0) ? 0.0 : T[j - 1];
      const double b = T[j];
      const double x = std::min(std::max(t, 0.0), b);
      const double len = std::max(0.0, x - a);
      if (len > 0.0) {
        I += f[j] * len;
      }
    }
    return -I;
  };
  auto pv_before = [&](const Bond &b, double cutoff,
                       const std::vector<double> &f,
                       std::size_t known_k) -> double {
    const auto times = b.payment_times();
    const auto cfs = b.cashflows();
    double pv = 0.0;
    for (std::size_t i = 0; i < times.size(); ++i) {
      if (times[i] <= cutoff) {
        pv = std::fma(cfs[i], std::exp(logD_known(times[i], f, known_k)), pv);
      }
    }
    return pv;
  };

  auto bisect = [&](auto &&g, double lo, double hi) -> double {
    double glo = g(lo), ghi = g(hi);
    int expand = 0;
    while (glo * ghi > 0.0 && expand < 24) {
      const double w = (hi - lo);
      lo -= w;
      hi += w;
      glo = g(lo);
      ghi = g(hi);
      ++expand;
    }
    if (glo * ghi > 0.0)
      throw std::runtime_error("bootstrap: cannot bracket root");
    for (int it = 0; it < maxit; ++it) {
      const double mid = 0.5 * (lo + hi);
      const double gm = g(mid);
      if (std::abs(gm) < tol || 0.5 * (hi - lo) < tol)
        return mid;
      if (glo * gm <= 0.0) {
        hi = mid;
        ghi = gm;
      } else {
        lo = mid;
        glo = gm;
      }
    }
    return 0.5 * (lo + hi);
  };

  std::vector<double> f_seg(bs.size(), 0.0);
  for (std::size_t k = 0; k < bs.size(); ++k) {
    const double Tk_1 = (k == 0) ? 0.0 : T[k - 1];
    const double Tk = T[k];

    const auto times = bs[k].payment_times();
    const auto cfs = bs[k].cashflows();

    const double price_mkt = prices[static_cast<std::size_t>(std::distance(
        bonds.begin(),
        std::find_if(bonds.begin(), bonds.end(), [&](const Bond &b) {
          return b.maturity() == bs[k].maturity();
        })))];

    const double target = price_mkt - pv_before(bs[k], Tk_1, f_seg, k);
    const double D_anchor = std::exp(logD_known(Tk_1, f_seg, k));

    auto g = [&](double f) -> double {
      double pv_seg = 0.0;
      for (std::size_t i = 0; i < times.size(); ++i) {
        const double t = times[i];
        if (t > Tk_1 && t <= Tk) {
          const double dt = t - Tk_1;
          pv_seg = std::fma(cfs[i], D_anchor * std::exp(-f * dt), pv_seg);
        }
      }
      return pv_seg - target;
    };
    f_seg[k] = bisect(g, f_lo, f_hi);
  }
  BootstrapResult br;
  br.f_seg = f_seg;
  br.pillars = build_pillars_from_f(f_seg, T, freq);
  return br;
}
} // namespace pricer