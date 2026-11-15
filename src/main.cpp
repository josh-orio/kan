#include <iostream>
#include <kan/kan.hpp>

using namespace kan;

int main(int argc, char *argv[]) {

  std::vector<std::string> args;

  for (int i = 1; i < argc; i++) {
    args.push_back(std::string(argv[i]));
  }

  // kan new, kan ls, kan link-commit (the post-commit hook) ... - no terminal UI
  if (args.size() >= 1 && cli::isCommand(args[0])) {
    return cli::run(args);
  }

  if (args.size() == 0) {
    args.push_back("");
  }
  else if (args[0] == "." || args[0] == "./") {
    args[0] = "";
  }

  try {
    KAN kan(args[0]);
    kan.start();
  }
  catch (const std::exception &e) {
    std::cerr << "\nERROR:\n" << e.what() << "\n\n";
    std::abort();
  }

  std::exit(0);

  time_t timestamp;
  time(&timestamp);

  return 0;
}