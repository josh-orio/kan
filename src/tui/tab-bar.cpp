#include <kan/tui.hpp>

namespace kan {
namespace tui {

TabBar::TabBar(std::vector<std::string> tabs) : _tabs(tabs) {}

TabBar &TabBar::width(int n) {
  _width = n;
  return *this;
}

TabBar &TabBar::Tab(int n) {
  _active_tab = n < 0 ? 0 : n > _tabs.size() ? _tabs.size() - 1 : n;
  return *this;
}

int TabBar::GetTab() { return _active_tab; }

void TabBar::TabIncrement() {
  _active_tab += 1;

  if (_active_tab == _tabs.size()) {
    _active_tab = 0;
  }
}

void TabBar::TabDecrement() {
  _active_tab -= 1;

  if (_active_tab < 0) {
    _active_tab = _tabs.size() - 1;
  }
}

void TabBar::render() {
  for (int i = 0; i < _tabs.size(); i++) {
    if (i > 0) {
      termui::terminal.write(" ");
    }

    if (i == _active_tab) {
      // termui::terminal.StyleStack(termui::Styles::bold);
      termui::terminal.StyleStack(::tui::stl({termui::SGR::Bold, termui::SGR::Underlined}));
      termui::terminal.write(" " + _tabs.at(i) + " ");
      termui::terminal.StylePop();
    }
    else {
      termui::terminal.write(" " + _tabs.at(i) + " ");
    }
  }
}

} // namespace tui
} // namespace kan
