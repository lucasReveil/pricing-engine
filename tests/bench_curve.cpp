#include "curve.hpp"
#include <charconv>
#include <chrono>
#include <cmath>
#include <ctime>
#include <exception>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>

using namespace pricer;
using Clock = std::chrono::steady_clock;

static double run_df_sweep(const Curve &c, const std::vector<double> &queries) {
  double acc = 0.0;
  for (double t : queries)
    acc += c.df(t);
  return acc;
}
int main() {
  std::vector<Pillar> ps;
  {
    const int N = 64;
    const double T = 30.0;
    ps.reserve(N);
    for (int i = 0; i < N; ++i) {
      double t = T * i / (N - 1);
      double r = 0.015 + 0.002 * t;
      double df = std::exp(-r * std::max(t, 1e-12));
      ps.push_back({t, df});
    }
  }
  std::vector<double> queries;
  {
    const size_t M = 2000000;
    queries.resize(M);
    std::mt19937_64 rng(123456);
    std::uniform_real_distribution<double> U(0.0, ps.back().t);
    for (auto &t : queries)
      t = U(rng);
  }
  Curve clog{ps, Interp::LogLinearDF};
  auto t0 = Clock::now();
  volatile double sink1 = run_df_sweep(clog, queries);
  auto t1 = Clock::now();

  Curve clin{ps, Interp::LinearDF};
  auto t2 = Clock::now();
  volatile double sink2 = run_df_sweep(clin, queries);
  auto t3 = Clock::now();

  const auto ns_log =
      std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
  const auto ns_lin =
      std::chrono::duration_cast<std::chrono::nanoseconds>(t3 - t2).count();
  std::cout << "acc_log=" << sink1 << " acc_lin=" << sink2 << "\n";
  std::cout << "LogLinearDF: " << ns_log << " ns total, "
            << (double)ns_log / queries.size() << " ns/call\n";
  std::cout << "LinearDF:    " << ns_lin << " ns total, "
            << (double)ns_lin / queries.size() << " ns/call\n";
}