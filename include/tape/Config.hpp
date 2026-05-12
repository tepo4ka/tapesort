#pragma once

#include <chrono>
#include <cstddef>

using Duration = std::chrono::nanoseconds;

struct TapeConfig {
  size_t RAMLimit{16 * 1024}; // Tape cells

  Duration ReadDelay{10};
  Duration WriteDelay{10};
  Duration MoveDelay{5};
  Duration RewindDelay{1000};
};
