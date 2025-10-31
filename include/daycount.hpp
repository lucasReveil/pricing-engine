#pragma once
#include <chrono>

namespace pricer {
using Days = std::chrono::days;
using SysDays = std::chrono::sys_days;
using Year = std::chrono::year;
using Month = std::chrono::month;
using Day = std::chrono::day;
#define ACT360 360.0;
struct Date {
  int y;
  unsigned m; // 1..12
  unsigned d; // 1..31
};

[[nodiscard]] SysDays to_sys_days(Date) noexcept;

[[nodiscard]] long days_between(Date a, Date b) noexcept;

[[nodiscard]] double year_fraction_ACT360(Date a, Date b) noexcept;
} // namespace pricer