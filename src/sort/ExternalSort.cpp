#include "sort/ExternalSort.hpp"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <print>
#include <queue>
#include <ranges>

ExternalSort::ExternalSort(SortConfig conf, ITape *in, ITape *out)
    : conf_{conf}, in_{in}, out_{out} {
}

void ExternalSort::Sort() {
  if (in_->Length() == 0) {
    // It is not our task to create the correct output tape
    assert(out_->Length() == 0);
    return;
  }
  out_->RewindLeft();
  MultiPassMerge(SortChunks(), out_);
}

std::vector<std::unique_ptr<ITape>> ExternalSort::SortChunks() {
  std::vector<std::unique_ptr<ITape>> runs;
  runs.reserve((in_->Length() + conf_.RAMCells - 1) / conf_.RAMCells);

  std::vector<TapeCell> ram(conf_.RAMCells);

  in_->RewindLeft();
  do {
    std::span chunk{in_->ReadChunk(ram)};
    std::ranges::sort(chunk, kComp);
    auto tape{conf_.NewTempTape(chunk.size())};
    auto written{tape->WriteChunk(chunk)};
    assert(written.size() == 0);
    runs.emplace_back(std::move(tape));
  } while (!in_->AtRightBound());

  return runs;
}

void ExternalSort::MultiPassMerge(std::vector<std::unique_ptr<ITape>> runs, ITape *out) {
  std::vector<std::unique_ptr<ITape>> next;

  while (runs.size() > conf_.RAMCells) {
    for (auto chunk : runs | std::views::chunk(conf_.RAMCells)) {
      std::vector<std::unique_ptr<ITape>> batch;
      std::ranges::move(chunk, std::back_inserter(batch));

      next.push_back(KWayMerge(std::move(batch)));
    }

    runs.swap(next);
    next.clear();
  }

  // Avoid creating a temporary tape as big as the output
  KWayMergeTo(std::move(runs), out);
}

std::unique_ptr<ITape> ExternalSort::KWayMerge(std::vector<std::unique_ptr<ITape>> runs) {
  assert(runs.size() <= conf_.RAMCells);

  auto const size{std::ranges::fold_left(runs, 0, [](auto acc, std::unique_ptr<ITape> const &t) {
    return acc + t->Length();
  })};
  auto out{conf_.NewTempTape(size)};

  KWayMergeTo(std::move(runs), out.get());
  return out;
}

void ExternalSort::KWayMergeTo(std::vector<std::unique_ptr<ITape>> runs, ITape *out) {
  assert(runs.size() <= conf_.RAMCells);

  auto const size{std::ranges::fold_left(runs, 0, [](auto acc, std::unique_ptr<ITape> const &t) {
    return acc + t->Length();
  })};
  assert(out->Length() >= size);

  auto const cmp{[](ITape *t1, ITape *t2) {
    // Min-heap, hence the negation
    return !kComp(t1->Read(), t2->Read());
  }};
  std::priority_queue<ITape *, std::vector<ITape *>, decltype(cmp)> heap(cmp);

  heap.push_range(runs | std::views::transform([](std::unique_ptr<ITape> &t) {
                    t->RewindLeft();
                    return t.get();
                  }));

  while (!heap.empty()) {
    auto *top{heap.top()};
    heap.pop();

    out->Write(top->Read());
    out->MoveRight();
    if (top->MoveRight()) {
      heap.push(top);
    }
  }
}
