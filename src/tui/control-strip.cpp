#include <kan/tui.hpp>

namespace kan {
namespace tui {

ControlStrip::ControlStrip(std::vector<std::pair<std::string, std::string>> control_map) : _control_map(control_map), _width(0) {}

ControlStrip &ControlStrip::width(uint n) {
  _width = n;
  return *this;
}

ControlStrip &ControlStrip::seperator(std::string s) {
  _seperator = s;
  return *this;
}

void ControlStrip::render() {
  int remaining_width = _width;

  for (int i = 0; i < _control_map.size(); i++) {
    if (i > 0) {
      if (remaining_width > 0) {
        // termui::terminal.CursorRight(1);
        termui::terminal.write(" ");
        remaining_width--;
      }
      if (remaining_width > 0) {
        termui::terminal.write(_seperator);
        remaining_width -= termui::visible_length(_seperator);
      }
      if (remaining_width > 0) {
        termui::terminal.CursorRight(1);
        remaining_width--;
      }
    }

    auto &control = _control_map.at(i);
    termui::terminal.write(control.first);
    remaining_width -= termui::visible_length(control.first);

    if (remaining_width > 0) {
      termui::terminal.CursorRight(1);
      remaining_width--;
    }

    termui::terminal.StyleStack(termui::Styles::faint);
    termui::terminal.write(control.second);
    termui::terminal.StylePop();
    remaining_width -= termui::visible_length(control.second);
  }

  while (remaining_width >= 0) {
    termui::terminal.write(" ");
    remaining_width--;
  }
}

} // namespace tui
} // namespace kan
