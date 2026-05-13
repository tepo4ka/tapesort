#include <expected>
#include <format>
#include <iostream>
#include <optional>
#include <print>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>

#include <lyra/lyra.hpp>

#define TOML_EXCEPTIONS 0
#include <toml++/toml.hpp>

#include "tape/Config.hpp"
#include "tape/FileTape.hpp"

template <typename TomlT, typename TargetT>
std::expected<void, std::string> ReadField(toml::table const &tbl, std::string_view key,
                                           TargetT &target) {
  auto const node{tbl[key]};
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

std::expected<TapeConfig, std::string> LoadConfig(toml::table const &tbl) {
  TapeConfig cfg{};
  return ReadField<size_t>(tbl, "RAMLimit, mit", cfg.RAMLimit)
      .and_then([&] {
        return ReadField<int64_t>(tbl, "ReadDelay", cfg.ReadDelay);
      })
      .and_then([&] {
        return ReadField<int64_t>(tbl, "WriteDelay", cfg.WriteDelay);
      })
      .and_then([&] {
        return ReadField<int64_t>(tbl, "MoveDelay", cfg.MoveDelay);
      })
      .and_then([&] {
        return ReadField<int64_t>(tbl, "RewindDelay", cfg.RewindDelay);
      })
      .transform([&] {
        return cfg;
      });
}

std::expected<TapeConfig, std::string> ResolveConfig(std::optional<std::string> const &conf_path) {
  if (conf_path) {
    auto const parse_result{toml::parse_file(*conf_path)};
    if (parse_result.failed()) {
      std::ostringstream error_desc;
      error_desc << parse_result.error();
      return std::unexpected{error_desc.str()};
    }
    return LoadConfig(parse_result.table());
  } else {
    return TapeConfig{};
  }
}

int main(int argc, char const **argv) {
  bool show_help{false};
  std::string input_path, output_path;
  std::optional<std::string> conf_path;

  auto const cli{
      lyra::help(show_help).description("External tape sorter.") |
      lyra::opt(input_path, "in.txt").required()["-i"]["--input"]("Input tape file") |
      lyra::opt(output_path, "out.txt").required()["-o"]["--output"]("Output tape file") |
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

  auto const conf{ResolveConfig(conf_path)};
  if (!conf) {
    std::cerr << conf.error();
    return 1;
  }

  {
    auto in_tape{FileTape::CreateNew(*conf, input_path, 100)};
    if (!in_tape) {
      std::cerr << in_tape.error();
      return 1;
    }

    for (auto i : std::views::iota(1)) {
      in_tape->Write(i);
      if (!in_tape->MoveRight()) {
        break;
      }
    }
  }

  auto out_tape{FileTape::OpenExisting(*conf, input_path)};
  if (!out_tape) {
    std::cerr << out_tape.error();
    return 1;
  }

  std::println("{}", out_tape->Length());
  do {
    std::println("{}", out_tape->Read());
  } while (out_tape->MoveRight());
}
