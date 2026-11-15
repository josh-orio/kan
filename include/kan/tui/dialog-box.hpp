#ifndef TUI_DIALOG_BOX_HPP
#define TUI_DIALOG_BOX_HPP

#include <termui/termui.hpp>

namespace kan {
namespace tui {

// a yes/no question answered with y or n, the labels are what the control strip shows (y archive · n cancel)
class DialogBox : public termui::Interface {
public:
  DialogBox(const std::string &title, const std::string &text, const std::string &yes = "yes", const std::string &no = "no");

  bool show(); // t = y pressed, f = n pressed

  static void notice(const std::string &title, const std::string &text); // nothing to answer, ↵ or esc closes it

private:
  std::string _title, _text, _yes, _no;

  size_t _width = 60, _height = 10;
  bool   _answer = false;

  void display();
  void process_input();
};

} // namespace tui
} // namespace kan

#endif
