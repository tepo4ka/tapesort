#pragma once

#include <cstdint>
#include <span>

using TapeCell = int32_t;

struct ReadChunkResult {
  std::span<TapeCell> data;
  bool eof;
};

// Magnetic tape simulator
class ITape {
public:
  virtual ~ITape() = default;

  virtual TapeCell Read() = 0;
  virtual void Write(TapeCell value) = 0;

  virtual bool MoveLeft() = 0;
  virtual bool MoveRight() = 0;

  virtual bool AtLeftBound() const = 0;
  virtual bool AtRightBound() const = 0;

  virtual uint64_t Length() const = 0;
  virtual uint64_t Position() const = 0;

public:
  // Following are helper functions which must use only operations defined above

  // Reads up to `to.size()` cells into `to`, advancing the tape as cell are read, and returns the
  // portion of `to` that was populated alongside with EOF indicator which is true iff no further
  // cells remain after the last returned element.
  //
  // The tape position is not restored.
  ReadChunkResult ReadChunk(std::span<TapeCell> to);

  // Write up to `from.size()` cells from `from` and returns the portion of `from` that wasn't
  // written. An empty span means that the write was successful.
  //
  // The tape position is not restored.
  std::span<TapeCell> WriteChunk(std::span<TapeCell> from);

  // Rewind the tape to its left boundary
  void RewindLeft();

  // Rewind the tape to its right boundary
  void RewindRight();
};
