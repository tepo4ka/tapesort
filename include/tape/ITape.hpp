#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

using TapeCell = int32_t;

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

  virtual size_t Length() const = 0;
  virtual size_t Position() const = 0;

public:
  // Following are helper functions which must use only operations defined above

  // Reads up to `to.size()` cells into `to`, advancing the tape as cell are read, and returns the
  // portion of `to` that was populated.
  //
  // The tape position is not restored.
  std::span<TapeCell> ReadChunk(std::span<TapeCell> to);

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
