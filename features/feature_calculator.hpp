#pragma once

#include "types.hpp"

namespace hft {
namespace features {

// EWMA Calculator
// Alpha = 2 / (N + 1)
class EWMA {
public:
  EWMA(double alpha) : alpha_(alpha), value_(0.0), initialized_(false) {}

  void update(double x) {
    if (!initialized_) {
      value_ = x;
      initialized_ = true;
    } else {
      value_ = alpha_ * x + (1.0 - alpha_) * value_;
    }
  }

  double value() const { return value_; }

private:
  double alpha_;
  double value_;
  bool initialized_;
};

} // namespace features
} // namespace hft
