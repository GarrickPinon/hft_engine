#pragma once

#include "types.hpp"
#include <atomic>

namespace hft::risk {

struct PositionLimit {
  core::SymbolId symbol_id;
  int64_t max_position; // Absolute value in base units
};

class KillSwitch {
public:
  static KillSwitch &instance() {
    static KillSwitch ks;
    return ks;
  }

  bool is_active() const { return active_.load(std::memory_order_acquire); }

  void trigger(const char *reason) {
    active_.store(true, std::memory_order_release);
    // In real system, we'd log reason here or broadcast alert
  }

  void reset() { active_.store(false, std::memory_order_release); }

private:
  std::atomic<bool> active_{false};
};

} // namespace hft::risk
