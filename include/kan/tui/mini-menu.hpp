#ifndef TUI_MINIMENU_HPP
#define TUI_MINIMENU_HPP

#include <kan/core.hpp>
#include <termui/termui.hpp>

namespace kan {
namespace tui {

class MiniMenu : termui::Interface {
private:
  std::string              _title;
  std::vector<std::string> _choices;
  unsigned int             _cursor;
  unsigned int             _visible_lines;
  unsigned int             _line_height = 1;
  unsigned int             _start_line;
  unsigned int             _line_spacing = 1;

  unsigned int _width = 40, _height = 15;

  void display();
  void process_input();
  void internal_update();

public:
  MiniMenu(std::string title, std::vector<std::string> choices);

  void         CursorUp(unsigned int count = 1);
  void         CursorDown(unsigned int count = 1);
  unsigned int getCursor();
  MiniMenu    &setCursor(unsigned int n);

  std::pair<termui::Interface::State, unsigned int> show();
};

} // namespace tui
} // namespace kan

#endif
