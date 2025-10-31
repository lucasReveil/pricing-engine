#include "daycount.hpp"
#include <chrono>

namespace pricer {
SysDays to_sys_days(Date d) noexcept {
  using namespace std::chrono;
  return sys_days{year(d.y) / month{d.m} / day{d.d}};
}
long days_between(Date a, Date b) noexcept {
  const auto sa = to_sys_days(a);
  const auto sb = to_sys_days(b);
  return (sb - sa).count();
}
double year_fraction_ACT360(Date a, Date b) noexcept {
  return static_cast<double>(days_between(a, b)) / ACT360;
}
} // namespace pricer