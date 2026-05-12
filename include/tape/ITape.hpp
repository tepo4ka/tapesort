#pragma once

#include <cstddef>
#include <cstdint>

using TapeCell = int32_t;

class ITape {
public:
  virtual ~ITape() = default;

  virtual TapeCell Read() const = 0;
  virtual void Write(TapeCell value) = 0;

  virtual void RewindLeft() = 0;
  virtual void RewindRight() = 0;

  virtual bool EOF() const = 0;
  virtual size_t Position() const = 0;
};
