#ifndef TUI_STATS_WINDOW_HPP
#define TUI_STATS_WINDOW_HPP

#include <kan/core.hpp>
#include <termui/termui.hpp>

namespace kan {
namespace tui {

// how the board is moving - throughput, cycle time and time spent in each column (kan stats in the terminal)
class StatsWindow : termui::Interface {
public:
  StatsWindow() = default;

  void show();

private:
  unsigned int _width = 80, _height = 40;
  int          _weeks = 8;

  void display();
  void process_input();
};

} // namespace tui
} // namespace kan

#endif
