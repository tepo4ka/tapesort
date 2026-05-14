#include "tape/ITape.hpp"

#include <span>

std::span<TapeCell> ITape::ReadChunk(std::span<TapeCell> to) {
  for (size_t i{0}; i < to.size(); ++i) {
    to[i] = Read();
    if (!MoveRight()) {
      return to.first(i + 1);
    }
  }

  return to;
}

std::span<TapeCell> ITape::WriteChunk(std::span<TapeCell> from) {
  for (size_t i{0}; i < from.size(); ++i) {
    Write(from[i]);
    if (!MoveRight()) {
      return from.subspan(i + 1);
    }
  }

  return from.subspan(from.size());
}

void ITape::RewindLeft() {
  while (MoveLeft()) {}
}

void ITape::RewindRight() {
  while (MoveRight()) {}
}
