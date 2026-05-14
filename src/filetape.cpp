#include <format>
#include <limits>
#include <numeric>
#include <print>
#include <random>

#include <lyra/lyra.hpp>

#include "tape/FileTape.hpp"
#include "tape/ITape.hpp"

struct RandCommand {
  uint64_t size{30};
  std::string path;

  void add_command(lyra::group &g) { // NOLINT
    g.add_argument(
        lyra::command("rand",
                      [this](lyra::group const &f) {
                        this->do_command(f);
                      })
            .help(std::format("Generate random tape with specified size (1 cell = {} bytes)",
                              sizeof(TapeCell)))
            .add_argument(lyra::arg(path, "out.bin").required())
            .add_argument(lyra::arg(size, "size").required()));
  }

  void do_command(lyra::group const &g) { // NOLINT
    auto tape{FileTape::CreateNew({}, path, size)};
    if (!tape) {
      std::cerr << tape.error() << '\n';
      return;
    }

    std::random_device rd;
    std::mt19937 gen{rd()};
    // std::uniform_int_distribution<TapeCell> dist{std::numeric_limits<TapeCell>::min(),
    //                                              std::numeric_limits<TapeCell>::max()};
    std::uniform_int_distribution<TapeCell> dist{-1000, 1000};

    do {
      tape->Write(dist(gen));
    } while (tape->MoveRight());
  }
};

struct PrintCommand {
  std::string path;

  void add_command(lyra::group &g) { // NOLINT
    g.add_argument(lyra::command("print",
                                 [this](lyra::group const &f) {
                                   this->do_command(f);
                                 })
                       .help("Print the file tape")
                       .add_argument(lyra::arg(path, "in.bin").required()));
  }

  void do_command(lyra::group const &g) { // NOLINT
    auto tape{FileTape::OpenExisting({}, path)};
    if (!tape) {
      std::cerr << tape.error() << '\n';
      return;
    }

    do {
      std::print("{} ", tape->Read());
    } while (tape->MoveRight());
    std::println();
  }
};

int main(int argc, char const **argv) {
  lyra::group global;
  bool show_help{false};
  global.add_argument(
      lyra::help(show_help).description("Helper utilities for working with file tapes."));

  lyra::group subcommands;
  subcommands.require(1, 1);

  RandCommand rand;
  rand.add_command(subcommands);

  PrintCommand print;
  print.add_command(subcommands);

  auto const cli{lyra::cli().add_argument(global).add_argument(subcommands)};
  auto const cli_result{cli.parse({argc, argv})};

  if (!cli_result.is_ok()) {
    std::cerr << cli << '\n';
    return 1;
  }

  if (show_help) {
    std::cout << cli << '\n';
    return 2;
  }
}
