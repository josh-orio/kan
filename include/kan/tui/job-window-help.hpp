#ifndef TUI_JOB_WINDOW_HELP_HPP
#define TUI_JOB_WINDOW_HELP_HPP

#include <kan/core.hpp>
#include <termui/termui.hpp>

namespace kan {
namespace tui {

class JobWindowHelp : termui::Interface {
private:
  uint _width = 80, _height = 40;

  std::map<std::string, std::string> _keyMap;

  void display();
  void process_input();

public:
  JobWindowHelp() = default;

  void show();
};

} // namespace tui
} // namespace kan

#endif
