#include <algorithm>
#include <cassert>
#include <iterator>
#include <queue>
#include <ranges>

#include "sort/ExternalSort.hpp"

ExternalSort::ExternalSort(SortConfig conf, ITape *in, ITape *out)
    : conf_{conf}, in_{in}, out_{out}, ram_(conf.RAMCells, {}) {
  in_->RewindLeft();
}

void ExternalSort::Sort() {
  in_->RewindLeft();
  auto out_tmp{MultiPassMerge(SortChunks())};

  out_->RewindLeft();
  out_tmp->RewindLeft();
  do {
    out_->Write(out_tmp->Read());
    out_tmp->MoveRight();
  } while (out_->MoveRight());
}

std::vector<std::unique_ptr<ITape>> ExternalSort::SortChunks() {
  std::vector<std::unique_ptr<ITape>> runs;
  runs.reserve(in_->Length() / conf_.RAMCells);

  std::span chunk{in_->ReadChunk(ram_)};
  while (chunk.size() > 0) {
    std::ranges::sort(chunk);
    auto tape{conf_.NewTempTape(chunk.size())};
    auto written{tape->WriteChunk(chunk)};
    assert(written.size() == 0);
    runs.emplace_back(std::move(tape));

    chunk = in_->ReadChunk(ram_);
  }

  return runs;
}

std::unique_ptr<ITape> ExternalSort::MultiPassMerge(std::vector<std::unique_ptr<ITape>> runs) {
  std::vector<std::unique_ptr<ITape>> next;
  next.reserve((runs.size() + conf_.RAMCells - 1) / conf_.RAMCells);

  while (runs.size() > 1) {
    for (auto chunk : runs | std::views::chunk(conf_.RAMCells)) {
      std::vector<std::unique_ptr<ITape>> batch;
      std::ranges::move(chunk, std::back_inserter(batch));

      next.push_back(KWayMerge(std::move(batch)));
    }

    runs.swap(next);
    next.clear();
  }

  return std::move(runs.front());
}

std::unique_ptr<ITape> ExternalSort::KWayMerge(std::vector<std::unique_ptr<ITape>> runs) {
  assert(runs.size() <= conf_.RAMCells);

  auto const size{std::ranges::fold_left(runs, 0, [](auto acc, std::unique_ptr<ITape> const &t) {
    return acc + t->Length();
  })};
  auto out{conf_.NewTempTape(size)};

  auto const cmp{[](ITape *t1, ITape *t2) {
    return t1->Read() > t2->Read();
  }};
  std::priority_queue<ITape *, std::vector<ITape *>, decltype(cmp)> heap(cmp);

  heap.push_range(runs | std::views::transform([](std::unique_ptr<ITape> &t) {
                    return t.get();
                  }));

  while (!heap.empty()) {
    auto *top{heap.top()};
    heap.pop();

    out->Write(top->Read());
    if (top->MoveRight()) {
      heap.push(top);
    }
  }

  return out;
}
