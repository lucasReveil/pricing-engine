#include "bond.hpp"
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <vector>
namespace pricer {
Bond::Bond(double notional, double coupon_rate, double maturity, int freq)
    : notional_(notional), coupon_rate_(coupon_rate), maturity_(maturity),
      frequency_(freq), nb_periods_(compute_periods(maturity, freq)) {
  if (frequency_ < 1) {
    throw std::invalid_argument("Frequency needs to be at least 1");
  }
  if (maturity <= 0) {
    throw std::invalid_argument("Maturity is time, has to be positive");
  }
  if (coupon_rate < 0) {
    throw std::invalid_argument("coupon rate should be >=0");
  }
  if (notional <= 0) {
    throw std::invalid_argument("notional should be positive");
  }
}

std::size_t Bond::compute_periods(double maturity, double frequency) {
  const double n = maturity * static_cast<double>(frequency);
  const double rn = std::round(n);
  if (std::fabs(n - rn) > 1e-9) {
    throw std::invalid_argument(
        "Bond: maturity*frequency must be (near) integer");
  }
  if (rn < 1.0)
    throw std::invalid_argument("Bond: needs at least one period");
  return static_cast<std::size_t>(rn);
}

std::vector<double> Bond::payment_times() const {
  std::vector<double> payment_times(nb_periods_);
  for (size_t i = 0; i < payment_times.size(); ++i) {
    payment_times[i] = static_cast<double>((i + 1.0) / frequency_);
  }
  return payment_times;
}
std::vector<double> Bond::cashflows() const {
  std::vector<double> cashflows(nb_periods_);
  double cf = coupon_rate_ * notional_ / frequency_;
  for (size_t i = 0; i < cashflows.size(); ++i) {
    cashflows[static_cast<size_t>(i)] = cf;
  }
  cashflows.back() = std::fma(coupon_rate_, notional_ / frequency_, notional_);
  return cashflows;
}

} // namespace pricer