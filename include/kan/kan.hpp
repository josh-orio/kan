#ifndef KAN_HPP
#define KAN_HPP

#include <filesystem>
#include <string>

#include <kan/cli.hpp>
#include <kan/core.hpp>
#include <kan/tui.hpp>

namespace kan {

class KAN : termui::Interface {
private:
  std::filesystem::path _path;
  std::string           _hook_note; // what init did about the post-commit hook, shown once the board is up

  void init();

public:
  KAN(std::string path);

  void start();
};

} // namespace kan

#endif
