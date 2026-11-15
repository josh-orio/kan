
#ifndef TUI_DIALOG_BOX_HPP
#define TUI_DIALOG_BOX_HPP

#include <termui/termui.hpp>

namespace kan {
namespace tui {

class DialogBox : public termui::Interface {
public:
  DialogBox(const std::string &title, const std::string &text, const std::string &affirmative, const std::string &negative);

  bool show(); // t = affirmative, f = negative

private:
  std::string _title, _text, _affirmative, _negative;

  size_t _width = 60, _height = 10;
  bool _selection; // t = affirmative selected, f = negative selected

  void display();
  void process_input();
};

} // namespace tui
} // namespace kan

#endif
