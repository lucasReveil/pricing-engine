// tests/test_curve.cpp
#include "curve.hpp"
#include "daycount.hpp"
#include <cmath>
#include <gtest/gtest.h>

using namespace pricer;

TEST(DayCount, Act360Basic) {
  Date d1{2025, 1, 1}, d2{2025, 1, 31};
  EXPECT_NEAR(year_fraction_ACT360(d1, d2), 30.0 / 360.0, 1e-12);
}

TEST(Curve, LogLinearDFInterpol) {
  std::vector<Pillar> ps = {{0.0, 1.0},
                            {0.5, std::exp(-0.02 * 0.5)},
                            {1.0, std::exp(-0.025 * 1.0)},
                            {2.0, std::exp(-0.03 * 2.0)}};
  Curve c{ps, Interp::LogLinearDF};

  // Interpolation: DF(0)=1
  EXPECT_NEAR(c.df(0.0), 1.0, 1e-12);

  // Monotonicité décroissante (taux positifs)
  EXPECT_LT(c.df(0.5), c.df(0.0));
  EXPECT_LT(c.df(1.0), c.df(0.5));

  // Forward instantané ~ taux spot local (ordre de grandeur)
  const double f1 = c.forward_instant(0.5);
  EXPECT_GT(f1, 0.0);
}

TEST(Curve, LinearDFInterpol) {
  std::vector<Pillar> ps = {{0.0, 1.0}, {1.0, 0.98}, {2.0, 0.94}};
  Curve c{ps, Interp::LinearDF};
  EXPECT_NEAR(c.df(0.5), 0.99, 1e-12); // linéaire entre 1.0 et 0.98
}

TEST(Curve, ForwardSimple) {
  std::vector<Pillar> ps = {
      {0.0, 1.0}, {1.0, std::exp(-0.03)}, {2.0, std::exp(-0.03 * 2.0)}};
  Curve c{ps, Interp::LogLinearDF};
  // forward 1Y-2Y ~ 3% en continu -> simple ~ (e^{0.03}-1) ≈ 0.03045 / 1 an
  const double f = c.forward_simple(1.0, 2.0);
  EXPECT_NEAR(f, std::expm1(0.03), 5e-4);
}
