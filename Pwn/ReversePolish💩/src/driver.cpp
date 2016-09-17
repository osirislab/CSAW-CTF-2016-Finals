#include "types.hpp"

#include "lexer.hpp"
#include "interp.hpp"

#include <fstream> // std::ifstream
#include <iostream> // std::cin, std::cerr

#include <cstdlib> // std::exit
#include <cstring> // std::strcmp

// EXIT CODES
// ----------
// 0  - success
// 1  - incorrect usage
// 2  - failed to open file
// 3  - parsing error
// 4  - runtime error
// 5  - internal compiler error
// -1 - bad allocation

auto main(int argc, char **argv) -> int {
  if (argc > 2) {
    std::cerr
      << "usage: "
      << (argc ? argv[0] : "program_name")
      << " [filename]\n";
    std::exit(1);
  } else if (argc == 2 && strcmp(argv[1], "-") != 0) {
    std::ifstream is(argv[1]);
    if (!is) {
      std::cerr << "failed to open file";
      std::exit(2);
    }

    auto interp = Interpreter(Lexer(is));
    interp.run();
  } else {
    auto interp = Interpreter(Lexer(std::cin));
    interp.run();
  }
}
