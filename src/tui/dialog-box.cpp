#include <kan/tui.hpp>

namespace kan {
namespace tui {

DialogBox::DialogBox(const std::string &title, const std::string &text, const std::string &yes, const std::string &no)
    : _title(title), _text(text), _yes(yes), _no(no) {}

void DialogBox::notice(const std::string &title, const std::string &text) { DialogBox(title, text, "ok", "").show(); }

bool DialogBox::show() {
  state = State::Continue;

  do {
    display();
    process_input();

  } while (state == State::Continue);

  return _answer;
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
  auto strip = _no.empty() ? kan::tui::ControlStrip({{"↵", _yes}}) : kan::tui::ControlStrip({{"y", _yes}, {"n", _no}});
  strip.width(_width - 4).render();

  termui::terminal.flush();
}

void DialogBox::process_input() {
  std::string inp = termui::terminal.read();

  if (_no.empty()) { // a notice
    if (inp == termui::key::ENTER || inp == termui::key::ESC || inp == "y" || inp == "n" || inp == "q") {
      _answer = true;
      state   = State::Select;
    }
    return;
  }

  if (inp == "y" || inp == "Y") {
    _answer = true;
    state   = State::Select;
  }
  else if (inp == "n" || inp == "N" || inp == termui::key::ESC) { // esc backs out, same as n
    _answer = false;
    state   = State::Select;
  }
}

} // namespace tui
} // namespace kan
