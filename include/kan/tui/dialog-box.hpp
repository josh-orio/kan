
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
  //   size_t aff_x1, aff_x2, aff_y; // these values hold bounds for the buttons
  //   size_t neg_x1, neg_x2, neg_y; // which is used to detect if 'pressed'
  bool _selection; // t = affirmative selected, f = negative selected
                   //   bool   reprint;               // flag indicates if reprint is required

  void display();
  void process_input();
};

} // namespace tui
} // namespace kan

#endif
