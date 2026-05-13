#include <cstdio>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <thread>

#include "tape/FileTape.hpp"
#include "tape/ITape.hpp"

FileTape::FileTape(FileTape &&other)
    : FileTape(other.conf_, other.path_, std::move(other.f_), other.capacity_,
               other.should_delete_) {
  head_ = other.head_;

  other.should_delete_ = false;
}

FileTape &FileTape::operator=(FileTape &&other) {
  if (this != &other) {
    conf_ = other.conf_;
    path_ = other.path_;
    f_ = std::move(other.f_);
    should_delete_ = other.should_delete_;
    head_ = other.head_;
    capacity_ = other.capacity_;

    other.should_delete_ = false;
  }
  return *this;
}

FileTape::~FileTape() {
  if (should_delete_) {
    std::error_code ec;
    std::filesystem::remove(path_, ec);
  }
}

std::expected<FileTape, std::string> FileTape::OpenExisting(TapeConfig conf,
                                                            std::filesystem::path const &path) {
  std::fstream f{path, std::ios_base::in | std::ios_base::out | std::ios_base::binary};
  if (!f.is_open()) {
    return std::unexpected{std::format("could not open the existing tape file {}", path.string())};
  }

  std::error_code ec;
  auto const capacity{std::filesystem::file_size(path, ec) / sizeof(TapeCell)};
  if (ec) {
    return std::unexpected{ec.message()};
  }

  return FileTape(conf, path, std::move(f), capacity);
}

std::expected<FileTape, std::string> FileTape::CreateNew(TapeConfig conf,
                                                         std::filesystem::path const &path,
                                                         uint64_t capacity, bool should_delete) {
  {
    std::ofstream create{path, std::ios::binary};
    if (!create) {
      return std::unexpected{std::format("could not create new tape file {}", path.string())};
    }
  }

  std::error_code ec;
  std::filesystem::resize_file(path, capacity * sizeof(TapeCell), ec);
  if (ec) {
    return std::unexpected{ec.message()};
  }

  std::fstream f{path, std::ios::in | std::ios::out | std::ios::binary};
  if (!f.is_open()) {
    return std::unexpected{std::format("could not open new tape file {}", path.string())};
  }

  return FileTape(conf, path, std::move(f), capacity, should_delete);
}

std::expected<FileTape, std::string> FileTape::CreateTemp(TapeConfig conf, uint64_t capacity) {
  return FileTape::CreateNew(conf, std::filesystem::path{std::tmpnam(nullptr)}, capacity, true);
}

TapeCell FileTape::Read() {
  std::this_thread::sleep_for(conf_.ReadDelay);
  TapeCell cell;
  f_.seekg(head_ * sizeof(TapeCell));
  f_.read(reinterpret_cast<char *>(&cell), sizeof(TapeCell));
  return cell;
}

void FileTape::Write(TapeCell value) {
  std::this_thread::sleep_for(conf_.WriteDelay);
  f_.seekp(head_ * sizeof(TapeCell));
  f_.write(reinterpret_cast<char const *>(&value), sizeof(TapeCell));
}

bool FileTape::MoveLeft() {
  std::this_thread::sleep_for(conf_.MoveDelay);
  if (AtLeftBound()) {
    return false;
  }

  head_--;
  return true;
}

bool FileTape::MoveRight() {
  std::this_thread::sleep_for(conf_.MoveDelay);
  if (AtRightBound()) {
    return false;
  }

  head_++;
  return true;
}

bool FileTape::AtLeftBound() const {
  return head_ == 0;
}

bool FileTape::AtRightBound() const {
  return head_ == capacity_ - 1;
}

size_t FileTape::Length() const {
  return capacity_;
}

size_t FileTape::Position() const {
  return head_;
}
