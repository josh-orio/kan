#include <iostream>
#include <kan/kan.hpp>

using namespace kan;

int main(int argc, char *argv[]) {
  //   std::cout << "Number of arguments: " << argc << std::endl;

  //   for (int i = 0; i < argc; i++) {
  //     std::cout << "argv[" << i << "] = " << argv[i] << std::endl;
  //   }

  // std::cout << std::filesystem::absolute(".") << std::endl;
  // std::cout << std::filesystem::absolute("") << std::endl;
  // std::cout << std::filesystem::absolute("./") << std::endl;
  // std::cout << std::filesystem::absolute("~/") << std::endl;

  // std::exit(0);

  std::vector<std::string> args;

  for (int i = 1; i < argc; i++) {
    args.push_back(std::string(argv[i]));
  }

  // std::cout << "kan ";
  // for(auto a:args) {
  //   std::cout<<a<<" ";
  // }
  // std::cout<<std::endl;
  // std::exit(0);

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

  // if (args.size() > 1) {
  //   throw std::invalid_argument("Too many arguments! - usage: kan <path>");
  // }
  else if (args.size() == 0) {
    args.push_back("");
  }
  else if (args[0] == "." || args[0] == "./") {
    args[0] = "";
  }

  try {
    // everything
    // run_program();
    KAN kan(args[0]);
    kan.start();
  }
  // catch (const nlohmann::json::exception &e) {
  //   std::cerr << "!!!! JSON EXCEPTION !!!!\n";
  //   std::cerr << e.what() << '\n';
  //   std::abort();
  // }
  catch (const std::exception &e) {
    std::cerr << "\nERROR:\n" << e.what() << "\n\n";
    std::abort();
  }

  std::exit(0);

  time_t timestamp;
  time(&timestamp);

  // Display the date and time represented by the timestamp
  // std::cout << ctime(&timestamp) ;//<< std::endl;

  // KAN kan;
  // kan.start();

  // Job j ("Job #1")

  // kan::Job job1("")

  // kan::tui::Cell sell ("Job #1",  UUIDv7::generate().to_string(), termui::Style(std::nullopt, termui::Colors::Blue ));
  // kan::tui::Cell sell2 ("Job #2",  UUIDv7::generate().to_string(), termui::Style(std::nullopt, termui::Colors::Blue ));

  // tui::Column col ("Fuck", {});

  // termui::terminal.ClearScreen();
  // termui::terminal.write(0,0,sell.width(18).render() + '\n');
  // termui::terminal.write(sell2.width(18).render() + '\n');
  // termui::terminal.write(col.width(10).height(10).render() + '\n');
  // termui::terminal.flush();
  // // std::cout <<  << std::endl;

  // auto tmp1 = UUIDv7::generate().to_string();
  // std::cout << tmp1 << std::endl;
  // std::cout << tmp1.substr(tmp1.size() - 6) << std::endl;

  // tmp1 = UUIDv7::generate().to_string();
  // std::cout << tmp1 << std::endl;
  // std::cout << tmp1.substr(tmp1.size() - 6) << std::endl;

  return 0;
}