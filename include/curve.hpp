#pragma once
#include <cassert>
// #include <cmath>
// #include <limits>
#include <span>
// #include <utility>
#include <vector>

namespace pricer {
enum class Interp : unsigned char { LinearDF, LogLinearDF };
struct Pillar {
  double t;
  double df;
};

class Curve {
public:
  Curve() = default;
  Curve(std::vector<Pillar> pillars, Interp method = Interp::LogLinearDF);
  [[nodiscard]] Interp method() const noexcept { return method_; }
  void set_method(Interp m) noexcept { method_ = m; }

  [[nodiscard]] std::span<const Pillar> pillars() const noexcept {
    return pillars_;
  }
  [[nodiscard]] double df(double t) const;
  [[nodiscard]] double forward_simple(double t1, double t2) const;
  [[nodiscard]] double forward_instant(double t) const;

private:
  std::vector<Pillar> pillars_;
  Interp method_{Interp::LogLinearDF};
  std::vector<double> logdf_cache_;
  [[nodiscard]] static int locate_segment(std::span<const Pillar> ps, double t);
  [[nodiscard]] double df_linear(double t, int i) const;
  [[nodiscard]] double df_loglin(double t, int i) const;
};

} // namespace pricer