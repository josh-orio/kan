#include <iostream>
#include <kan/kan.hpp>

using namespace kan;

int main(int argc, char *argv[]) {

  std::vector<std::string> args;

  for (int i = 1; i < argc; i++) {
    args.push_back(std::string(argv[i]));
  }

  if (args.size() >= 1 && (args[0] == "help" || args[0] == "--help" || args[0] == "-h")) {
    std::cout << "kan - a kanban board that lives in your git repo\n"
                 "\n"
                 "usage:\n"
                 "  kan [path]                  open the board in <path>/.kan (default: current directory),\n"
                 "                              creating a new board there if none exists\n"
                 "  kan link-commit <hash>      link a commit to every job its message references with #N\n"
                 "                              (run by the post-commit hook, e.g. \"fix #3\")\n"
                 "  kan help                    show this guide\n";
    return 0;
  }

  // called by the post-commit hook, stays away from KAN as there may be no terminal to set up
  if (args.size() == 2 && args[0] == "link-commit") {
    try {
      board = Board(".kan");
      board.loadBoard();
      board.loadJobs();

      hooks::postCommit(board, args[1]);
    }
    catch (const std::exception &e) {
      std::cerr << "kan link-commit: " << e.what() << "\n";
      return 1;
    }

    return 0;
  }

  else if (args.size() == 0) {
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