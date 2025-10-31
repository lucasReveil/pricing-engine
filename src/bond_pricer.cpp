#include "bond_pricer.hpp"
#include <cstddef>
#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/view/zip.hpp>
namespace pricer {
BondPricer::BondPricer(const Curve &curve) : curve_(curve) {}
double BondPricer::priceTradi(const Bond &bond) const {
  double P = 0;
  const auto cashflows = bond.cashflows();
  const auto times = bond.payment_times();
  for (size_t i = 0; i < times.size(); ++i) {
    P = std::fma(cashflows[i], curve_.df(times[i]), P);
  }
  return P;
}
double BondPricer::price(const Bond &bond) const {
  const auto cashflows = bond.cashflows();
  const auto times = bond.payment_times();
  return ranges::accumulate(ranges::views::zip(times, cashflows), 0.0,
                            std::plus<>{}, [&](auto pair) {
                              const auto [t, cf] = pair;
                              return cf * curve_.df(t);
                            });
}
} // namespace pricer