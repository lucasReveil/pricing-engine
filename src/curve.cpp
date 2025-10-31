#include "curve.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <vector>

namespace pricer {
Curve::Curve(std::vector<Pillar> pillars, Interp method)
    : pillars_(std::move(pillars)), method_((method)) {
  if (pillars_.size() < 2) {
    throw std::invalid_argument("Curve: need >= 2 pillars");
  }

  std::sort(pillars_.begin(), pillars_.end(),
            [](const Pillar &a, const Pillar &b) { return a.t < b.t; });
  for (const auto &p : pillars_) {
    if (!(p.df > 0.0) || !std::isfinite(p.df)) {
      throw std::invalid_argument("Curve: DF must be finite and > 0");
    }
  }
  if (method_ == Interp::LogLinearDF) {
    logdf_cache_.resize(pillars_.size());
    for (size_t i = 0; i < pillars_.size(); ++i) {
      logdf_cache_[i] = std::log(std::max(1e-300, pillars_[i].df));
    }
  }
}
int Curve::locate_segment(std::span<const Pillar> ps, double t) {
  if (t < ps.front().t) {
    return -1;
  }
  if (t > ps.back().t) {
    return static_cast<int>(ps.size()) - 1;
  }
  auto it =
      std::upper_bound(ps.begin(), ps.end(), t,
                       [](double val, const Pillar &p) { return val < p.t; });
  int i = static_cast<int>(std::distance(ps.begin(), it)) - 1;
  return std::clamp(i, 0, static_cast<int>(ps.size()) - 2);
}
double Curve::df_linear(double t, int i) const {
  const auto &p0 = pillars_[static_cast<std::size_t>(i)];
  const auto &p1 = pillars_[static_cast<std::size_t>(i + 1)];
  const double w = (t - p0.t) / (p1.t - p0.t);
  return std::fma(w, (p1.df - p0.df), p0.df); // p0.df +w * (p1.df-p0.df)
}
double Curve::df_loglin(double t, int i) const {
  const auto &p0 = pillars_[static_cast<std::size_t>(i)];
  const auto &p1 = pillars_[static_cast<std::size_t>(i + 1)];
  const double dt = p1.t - p0.t;
  if (dt <= 0.0) {
    return p1.df;
  }
  const double log0 = logdf_cache_.empty()
                          ? std::log(std::max(1e-300, p0.df))
                          : logdf_cache_[static_cast<size_t>(i)];
  const double log1 = logdf_cache_.empty()
                          ? std::log(std::max(1e-300, p1.df))
                          : logdf_cache_[static_cast<size_t>(i + 1)];
  const double w = (t - p0.t) / dt;
  return std::exp(std::fma(w, (log1 - log0), log0)); // exp(log0+w*(log1-log0))
}
double Curve::df(double t) const {
  const int i = locate_segment(pillars_, t);
  if (i < 0) { // left ; hold-fwd log-lin
    if (method_ == Interp::LinearDF) {
      const auto &p0 = pillars_[0];
      const double slope = (pillars_[1].df - p0.df) / (pillars_[1].t - p0.t);
      return std::max(0.0, std::fma((t - p0.t), slope, p0.df));
    } else {
      const auto &p0 = pillars_[0];
      const double r0 = -std::log(p0.df) / std::max(p0.t, 1e-12);
      return std::exp(-r0 * t);
    }
  }
  if (i >= static_cast<int>(pillars_.size()) - 1) { // right ; local tan
    const auto &pnm1 = pillars_[pillars_.size() - 2];
    const auto &pn = pillars_[pillars_.size() - 1];
    if (method_ == Interp::LinearDF) {
      const double slope = (pn.df - pnm1.df) / (pn.t - pnm1.t);
      return std::max(0.0, std::fma((t - pn.t), slope, pn.df));
    } else {
      const double r = -std::log(pn.df / pnm1.df) / (pn.t - pnm1.t);
      return pn.df * std::exp(-r * (t - pn.t));
    }
  }
  return (method_ == Interp::LinearDF) ? df_linear(t, i) : df_loglin(t, i);
}
double Curve::forward_simple(double t1, double t2) const {
  if (!(t2 > t1)) {
    throw std::invalid_argument("forward_simple:need t2>t1");
  }
  const double df1 = df(t1);
  const double df2 = df(t2);
  return (df1 / df2 - 1.0) / (t2 - t1);
}
double Curve::forward_instant(double t) const {
  constexpr double eps = 1e-5;
  const double tL = std::max(0.0, t - eps);
  const double tR = t + eps;
  const double lL = std::log(std::max(1e-300, df(tL)));
  const double lR = std::log(std::max(1e-300, df(tR)));
  return -(lR - lL) / (tR - tL);
}
} // namespace pricer