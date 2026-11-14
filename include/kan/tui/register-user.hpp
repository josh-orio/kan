#ifndef TUI_REGISTER_USER_HPP
#define TUI_REGISTER_USER_HPP

#include <termui/termui.hpp>

namespace kan {
namespace tui {

class RegisterUser : public termui::Interface {
public:
  RegisterUser(const std::string &title, const std::string &text);

  bool show(); // t = yes, f = no

private:
  std::string _title, _text;

  size_t _width = 60, _height = 10;
  bool _selection; // t = yes, f = no

  void display();
  void process_input();
};

} // namespace tui
} // namespace kan

#endif
