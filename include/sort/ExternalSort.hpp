#pragma once

#include <memory>
#include <span>
#include <vector>

#include "tape/ITape.hpp"

#include "sort/Config.hpp"

// External merge sort with multi-pass K-way merge
class ExternalSort {
public:
  ExternalSort(SortConfig conf, ITape *in, ITape *out);

  void Sort();

private:
  // Splits the input tape into sorted runs and writes each run to a temporary output tape. Each run
  // is formed by up to `RAMCells` elements from the input tape.
  //
  // Returns the created run tapes.
  std::vector<std::unique_ptr<ITape>> SortChunks();

  std::unique_ptr<ITape> MultiPassMerge(std::vector<std::unique_ptr<ITape>> runs);

  std::unique_ptr<ITape> KWayMerge(std::vector<std::unique_ptr<ITape>> runs);

private:
  SortConfig conf_;

  // Own only temporary tapes, but do not own input and output tapes
  ITape *in_;
  ITape *out_;

  std::vector<std::unique_ptr<ITape>> tmps_;
  std::vector<ITape *> runs_;

  // FIXME: ensure the size?
  std::vector<TapeCell> ram_;
};
