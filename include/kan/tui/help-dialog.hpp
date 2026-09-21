#ifndef TUI_HELP_DIALOG_HPP
#define TUI_HELP_DIALOG_HPP

#include <kan/core.hpp>
#include <termui/termui.hpp>

namespace kan {
namespace tui {

class HelpDialog : termui::Interface {
private:
  uint _width = 80, _height = 40;

  void display();
  void process_input();

public:
  HelpDialog() = default;

  void show();
};

} // namespace tui
} // namespace kan

#endif
