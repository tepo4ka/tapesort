#pragma once

#include <cstddef>
#include <cstdint>

using TapeCell = int32_t;

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
};
