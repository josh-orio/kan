#include <kan/tui.hpp>

namespace kan {
namespace tui {

RegisterUser::RegisterUser(const std::string &title, const std::string &text) : _title(title), _text(text), _selection(false) {}

bool RegisterUser::show() {
  state = State::Continue;

  do {
    display();
    process_input();

  } while (state == State::Continue);

  return _selection;
}

void RegisterUser::display() {
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
                             {"y", "yes"},
                             {"n", "no"},
                         })
      .width(_width - 4)
      .render();

  termui::terminal.flush();
}

void RegisterUser::process_input() {
  std::string inp = termui::terminal.read();

  if (inp == "y" || inp == "Y") {
    _selection = true;
    state      = State::Select;
  }
  else if (inp == "n" || inp == "N") {
    _selection = false;
    state      = State::Select;
  }
}

} // namespace tui
} // namespace kan
