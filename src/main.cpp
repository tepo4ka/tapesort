#include <expected>
#include <format>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>

#include <lyra/lyra.hpp>

#define TOML_EXCEPTIONS 0
#include <toml++/toml.hpp>

#include "tape/Config.hpp"
#include "tape/FileTape.hpp"

#include "sort/Config.hpp"
#include "sort/ExternalSort.hpp"

std::expected<toml::table, std::string> ParseTOML(std::string_view path) {
  auto const result{toml::parse_file(path)};
  if (result.failed()) {
    std::ostringstream os;
    os << result.error();
    return std::unexpected{os.str()};
  }

  return result.table();
}

template <typename TomlT, typename TargetT>
std::expected<void, std::string> ReadField(toml::table const &tbl, std::string_view key,
                                           TargetT &target) {
  auto const node{tbl.at_path(key)};
  if (!node) {
    return {};
  }

  auto const value{node.value<TomlT>()};
  if (!value) {
    return std::unexpected{std::format("Incorrect type for the key {}", key)};
  }

  target = TargetT{*value};
  return {};
}

std::expected<TapeConfig, std::string> LoadTapeConfig(toml::table const &tbl) {
  TapeConfig cfg{};
  return ReadField<size_t>(tbl, "tape.read_delay", cfg.ReadDelay)
      .and_then([&] {
        return ReadField<int64_t>(tbl, "tape.write_delay", cfg.WriteDelay);
      })
      .and_then([&] {
        return ReadField<int64_t>(tbl, "tape.move_delay", cfg.MoveDelay);
      })
      .transform([&] {
        return cfg;
      });
}

std::expected<SortConfig, std::string> LoadSortConfig(toml::table const &tbl) {
  SortConfig cfg{};
  return ReadField<int>(tbl, "sort.ram", cfg.RAMCells).transform([&] {
    return cfg;
  });
}

int main(int argc, char const **argv) {
  bool show_help{false};
  std::string input_path, output_path;
  std::optional<std::string> conf_path;

  auto const cli{
      lyra::help(show_help).description("External tape sorter.") |
      lyra::opt(input_path, "in.bin").required()["-i"]["--input"]("Input tape file") |
      lyra::opt(output_path, "out.bin").required()["-o"]["--output"]("Output tape file") |
      lyra::opt(conf_path, "conf.toml").optional()["-c"]["--config"]("Tape config file")};
  auto const cli_result{cli.parse({argc, argv})};

  if (!cli_result.is_ok()) {
    std::cerr << cli << '\n';
    return 1;
  }

  if (show_help) {
    std::cout << cli << '\n';
    return 2;
  }

  std::expected<TapeConfig, std::string> tape_conf{};
  std::expected<SortConfig, std::string> sort_conf{};

  if (conf_path) {
    auto const conf_tbl{ParseTOML(*conf_path)};
    tape_conf = conf_tbl.and_then(LoadTapeConfig);
    sort_conf = conf_tbl.and_then(LoadSortConfig);
  }

  if (!tape_conf) {
    std::cerr << tape_conf.error() << '\n';
    return 1;
  }

  if (!sort_conf) {
    std::cerr << sort_conf.error() << '\n';
    return 1;
  }

  sort_conf->NewTempTape = [tape_conf = *tape_conf](uint64_t capacity) -> std::unique_ptr<ITape> {
    auto tape{FileTape::CreateTemp(tape_conf, capacity)};
    if (!tape) {
      // There is no way to recover if we can't write to the temporary tape
      throw std::runtime_error(tape.error());
    }

    return std::make_unique<FileTape>(std::move(*tape));
  };

  auto in_tape{FileTape::OpenExisting(*tape_conf, input_path)};
  if (!in_tape) {
    std::cerr << in_tape.error() << '\n';
    return 1;
  }

  auto out_tape{FileTape::CreateNew(*tape_conf, output_path, in_tape->Length())};
  if (!out_tape) {
    std::cerr << out_tape.error() << '\n';
    return 1;
  }

  ExternalSort sort{*sort_conf, &in_tape.value(), &out_tape.value()};
  sort.Sort();
}
