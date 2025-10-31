#pragma once
#include <cstddef>
#include <vector>
namespace pricer {
class Bond {
public:
  Bond(double notional, double coupon_rate, double maturity, int freq);
  double notional() const noexcept { return notional_; }
  double coupon_rate() const noexcept { return coupon_rate_; }
  double maturity() const noexcept { return maturity_; }
  int frequency() const noexcept { return frequency_; }
  size_t nb_periods() const noexcept { return nb_periods_; }
  [[nodiscard]] std::vector<double> payment_times() const;
  [[nodiscard]] std::vector<double> cashflows() const;

private:
  double notional_;
  double coupon_rate_;
  double maturity_;
  int frequency_;
  size_t nb_periods_;
  static std::size_t compute_periods(double maturity, double freq);
};
} // namespace pricer