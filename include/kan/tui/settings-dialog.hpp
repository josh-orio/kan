#ifndef TUI_SETTINGS_WINDOW_HPP
#define TUI_SETTINGS_WINDOW_HPP

#include <kan/core.hpp>
#include <termui/termui.hpp>

namespace kan {
namespace tui {

class SettingsDialog : termui::Interface {
private:
  uint _width = 64, _height = 8;

  void display();
  void process_input();

public:
  SettingsDialog() = default;

  void show();
};

} // namespace tui
} // namespace kan

#endif
