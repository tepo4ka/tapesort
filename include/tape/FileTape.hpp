#pragma once

#include <concepts>
#include <expected>
#include <filesystem>
#include <fstream>

#include "Config.hpp"
#include "ITape.hpp"

// Binary file format: <TapeCell><TapeCell>...<TapeCell>
class FileTape : ITape {
public:
  FileTape() = delete;

  static std::expected<FileTape, std::string> OpenExisting(TapeConfig conf,
                                                           std::filesystem::path const &path);

  // If CreateNew overwrites an existing file, reading unwritten cells is undefined
  static std::expected<FileTape, std::string>
  CreateNew(TapeConfig conf, std::filesystem::path const &path, uint64_t capacity);

  static std::expected<FileTape, std::string> CreateTemp(TapeConfig conf, uint64_t capacity);

  TapeCell Read() override;
  void Write(TapeCell value) override;

  bool MoveLeft() override;
  bool MoveRight() override;

  bool AtLeftBound() const override;
  bool AtRightBound() const override;

  size_t Length() const override;
  size_t Position() const override;

private:
  template <typename S>
    requires std::convertible_to<S, std::fstream const &>
  FileTape(TapeConfig conf, S &&f, uint64_t capacity)
      : conf_{conf}, f_{std::forward<S>(f)}, capacity_{capacity} {
  }

  TapeConfig conf_;

  std::fstream f_;
  size_t head_{}, capacity_{};  // FIXME: which type?
};
