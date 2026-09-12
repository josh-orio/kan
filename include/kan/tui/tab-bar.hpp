#ifndef TUI_TAB_BAR_HPP
#define TUI_TAB_BAR_HPP

#include <kan/core.hpp>
#include <termui/termui.hpp>

namespace kan {
namespace tui {

class TabBar {
private:
  std::vector<std::string> _tabs;
  int                      _width, _height = 1;
  int                      _active_tab = 0;

public:
  TabBar(std::vector<std::string> tabs);

  TabBar &width(int n);

  TabBar &Tab(int n);
  int     GetTab();

  void TabIncrement();
  void TabDecrement();

  void render();
};

} // namespace tui
} // namespace kan

#endif
