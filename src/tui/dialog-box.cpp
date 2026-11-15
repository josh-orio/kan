#include <kan/tui.hpp>

namespace kan {
namespace tui {

DialogBox::DialogBox(const std::string &title, const std::string &text, const std::string &affirmative, const std::string &negative)
    : _title(title), _text(text), _affirmative(affirmative), _negative(negative), _selection(false) {} // start on the negative (safe) option

bool DialogBox::show() {
  state = State::Continue;

  do {
    display();
    process_input();

  } while (state == State::Continue);

  return state == State::Select && _selection; // esc is always a no
}

void DialogBox::display() {
  auto terminal_half_w = termui::terminal.halfWidth();
  auto terminal_half_h = termui::terminal.halfHeight();

  auto window_start_row = terminal_half_h - (_height / 2);
  auto window_start_col = terminal_half_w - (_width / 2);

  // box
  termui::terminal.MoveCursor(window_start_row, window_start_col);
  termui::Box(_width, _height, termui::Borders::rounded, termui::Styles::faint).render();

  // title
  termui::terminal.MoveCursor(window_start_row + 1, window_start_col + 2);
  termui::Text(_title, termui::Styles::bold).width(_width - 4).height(1).render();

  // text
  termui::terminal.MoveCursor(window_start_row + 3, window_start_col + 2);
  termui::Text(_text).width(_width - 4).height(4).render();

  // control bar
  termui::terminal.MoveCursor(window_start_row + _height - 2, window_start_col + 2);
  kan::tui::ControlStrip({
                             {"esc", "close"},
                             {"←→", "navigate"},
                             {"↵", "select"},
                         })
      .width(_width - 4 - 1 - 8 - 1 - 8 - 1)
      .render();

  // buttons

  termui::terminal.write(" ");

  // delete button
  termui::Button(_affirmative)
      .focus_style(termui::Style({termui::SGR::Bold, termui::SGR::ReverseVideo}))
      .blur_style(termui::Style())
      .width(6)
      .setFocus(_selection)
      .render();

  termui::terminal.write(" ");

  // cancel button
  termui::Button(_negative)
      .focus_style(termui::Style({termui::SGR::Bold, termui::SGR::ReverseVideo}))
      .blur_style(termui::Style())
      .width(6)
      .setFocus(!_selection)
      .render();

  termui::terminal.flush();
}

void DialogBox::process_input() {
  std::string inp = termui::terminal.read();

  if (inp == termui::key::ESC) {
    state = termui::Interface::State::Exit;
  }
  else if (inp == termui::key::ENTER) {
    state = State::Select;
  }
  else if (inp == termui::key::L_ARROW) {
    _selection = true;
  }
  else if (inp == termui::key::R_ARROW) {
    _selection = false;
  }
}

} // namespace tui
} // namespace kan
