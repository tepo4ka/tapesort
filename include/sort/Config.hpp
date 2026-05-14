#pragma once

#include <functional>
#include <memory>

#include "tape/ITape.hpp"

struct SortConfig {
  int RAMCells{100};

  // Create new temporary tape with specified capacity
  std::function<std::unique_ptr<ITape>(uint64_t)> NewTempTape;
};
