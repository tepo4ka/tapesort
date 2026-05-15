#include <format>
#include <print>

#include <lyra/lyra.hpp>

#include "tape/FileTape.hpp"

struct PrintCommand {
  std::string path;

  void add_command(lyra::group &g) { // NOLINT
    g.add_argument(lyra::command("print",
                                 [this](lyra::group const &f) {
                                   this->do_command(f);
                                 })
                       .help("Print the file tape")
                       .add_argument(lyra::arg(path, "tape.bin").required()));
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
