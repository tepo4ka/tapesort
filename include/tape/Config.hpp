#pragma once

#include <chrono>

using Duration = std::chrono::nanoseconds;

struct TapeConfig {
  Duration ReadDelay{10};
  Duration WriteDelay{10};
  Duration MoveDelay{5};
};
