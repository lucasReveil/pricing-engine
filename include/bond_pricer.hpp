#pragma once
#include "bond.hpp"
#include "curve.hpp"
namespace pricer {
class BondPricer {
public:
  explicit BondPricer(const Curve &curve);

  double price(const Bond &bond) const;
  double priceTradi(const Bond &bond) const;
  double yield_to_maturity(const Bond &bond, double target_price) const;
  double macaulay_duration(const Bond &bond) const;
  double convexity(const Bond &bond) const;

private:
  const Curve &curve_;
};

} // namespace pricer