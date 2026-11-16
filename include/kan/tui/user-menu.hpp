#ifndef TUI_USERMENU_HPP
#define TUI_USERMENU_HPP

#include <kan/core.hpp>
#include <termui/termui.hpp>

namespace kan {
namespace tui {

class UserMenu : termui::Interface {
private:
  std::string       _title = "Assign User";
  std::vector<User> _users;
  std::vector<User> _filtered; // _users narrowed down by _query
  std::string       _query;
  unsigned int      _cursor;
  unsigned int      _visible_lines;
  unsigned int      _line_height = 1;
  unsigned int      _start_line;
  unsigned int      _line_spacing = 1;

  unsigned int _width = 60, _height = 15;

  void display();
  void process_input();
  void internal_update();
  void apply_filter();

public:
  UserMenu();

  void         CursorUp(unsigned int count = 1);
  void         CursorDown(unsigned int count = 1);
  unsigned int getCursor();
  UserMenu    &setCursor(unsigned int n);
  UserMenu    &setCursor(const User &user); // hovers user if they're in the list

  std::pair<termui::Interface::State, User> show();
};

} // namespace tui
} // namespace kan

#endif