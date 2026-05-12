#pragma once

#include "Config.hpp"
#include "ITape.hpp"

class FileTape {
public:
  FileTape(TapeConfig const &conf);

  TapeCell Read() const;
  void Write(TapeCell value);

  void RewindLeft();
  void RewindRight();

  bool EOF() const;
  size_t Position() const;
};
