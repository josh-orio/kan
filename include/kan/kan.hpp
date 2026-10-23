#ifndef KAN_HPP
#define KAN_HPP

#include <filesystem>
#include <string>

#include <kan/core.hpp>
#include <kan/tui.hpp>

namespace kan {

class KAN : termui::Interface {
private:
  std::filesystem::path _path;
  // kan::tui::Board       _renderer;

  void init();

public:
  KAN(std::string path);

  void start();
};

} // namespace kan

#endif
