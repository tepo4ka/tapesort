#pragma once

#include <concepts>
#include <expected>
#include <filesystem>
#include <fstream>

#include "Config.hpp"
#include "ITape.hpp"

// Binary file format: <TapeCell><TapeCell>...<TapeCell>.
//
// This class owns its file stream.
class FileTape : public ITape {
public:
  FileTape() = delete;
  FileTape(FileTape &) = delete;
  FileTape(FileTape &&);
  FileTape &operator=(FileTape &&);
  FileTape &operator=(FileTape &) = delete;
  ~FileTape();

  static std::expected<FileTape, std::string> OpenExisting(TapeConfig conf,
                                                           std::filesystem::path const &path);

  // If CreateNew overwrites an existing file, reading unwritten cells is undefined
  static std::expected<FileTape, std::string> CreateNew(TapeConfig conf,
                                                        std::filesystem::path const &path,
                                                        uint64_t capacity,
                                                        bool should_delete = false);

  static std::expected<FileTape, std::string> CreateTemp(TapeConfig conf, uint64_t capacity);

  TapeCell Read() override;
  void Write(TapeCell value) override;

  bool MoveLeft() override;
  bool MoveRight() override;

  bool AtLeftBound() const override;
  bool AtRightBound() const override;

  uint64_t Length() const override;
  uint64_t Position() const override;

private:
  template <typename S>
    requires std::convertible_to<S, std::fstream const &>
  FileTape(TapeConfig conf, std::filesystem::path path, S &&f, uint64_t capacity,
           bool should_delete = false)
      : conf_{conf}, path_{path}, f_{std::forward<S>(f)}, should_delete_{should_delete},
        capacity_{capacity} {
  }

  TapeConfig conf_;

  std::filesystem::path path_;
  std::fstream f_;

  // Indicates whether the file at `path_` should be deleted when this object is destroyed.
  // Used primarily for temporary files, because this class owns them too.
  //
  // Ideally, `tmpfile(3)` would be used, since it provides automatic cleanup, but there is no
  // portable way to construct `std::fstream` from `FILE *`.
  bool should_delete_{false};

  uint64_t head_{}, capacity_;
};
