#pragma once

#include <memory>
#include <vector>

#include "tape/ITape.hpp"

#include "sort/Config.hpp"

// External merge sort with multi-pass K-way merge
class ExternalSort {
public:
  ExternalSort(SortConfig conf, ITape *in, ITape *out);

  void Sort();

private:
  // Because we are dealing with temporary tapes, each method takes ownership of the provided tape
  // set, which is destroyed by the end of processing.
  //
  // Each method is responsible for managing tape state, including rewinding tapes when required.

  // Splits the input tape into sorted runs and writes each run to a temporary output tape. Each run
  // is formed by up to `RAMCells` elements from the input tape.
  //
  // Returns the created run tapes.
  std::vector<std::unique_ptr<ITape>> SortChunks();

  // Perform external k-way merge over a set of sorted input `runs`, reducing them iteratively until
  // a single sorted output tape remains. The result is written directly into `out`.
  //
  // Uses O(RAMCells) RAM, which is achieved by multiple merge passes.
  void MultiPassMerge(std::vector<std::unique_ptr<ITape>> runs, ITape *out);

  // Merge sorted `runs` into a single sorted temporary output tape. Only one element from each tape
  // is stored in the RAM, thus requiring `runs.size() <= RAMCells`.
  std::unique_ptr<ITape> KWayMerge(std::vector<std::unique_ptr<ITape>> runs);

  // Like `KWayMerge`, but writes into `out` instead of constructing the new temporary tape.
  void KWayMergeTo(std::vector<std::unique_ptr<ITape>> runs, ITape* out);

private:
  SortConfig conf_;

  // Own only temporary tapes, but do not own input and output tapes
  ITape *in_;
  ITape *out_;
};
