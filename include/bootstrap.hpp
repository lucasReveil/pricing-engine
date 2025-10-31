#pragma once
#include "bond.hpp"
#include "curve.hpp"
#include <vector>

namespace pricer {
struct BootstrapResult {
  std::vector<double> f_seg;
  std::vector<Pillar> pillars;
};
BootstrapResult boostrap_piecewise_forward(const std::vector<Bond> &bonds,
                                           const std::vector<double> &prices,
                                           double f_lo = -0.05,
                                           double f_hi = 0.5,
                                           double tol = 1e-12, int maxit = 200);
inline Curve make_curve_from_bootstrap(const BootstrapResult &br) {
  return Curve{br.pillars, Interp::LogLinearDF};
}
} // namespace pricer