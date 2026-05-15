#pragma once

#include <chrono>

using Duration = std::chrono::milliseconds;

struct TapeConfig {
  Duration ReadDelay{10};
  Duration WriteDelay{20};
  Duration MoveDelay{5};
};
