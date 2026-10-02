#include <kan/tui.hpp>

namespace kan {
namespace tui {

void SettingsDialog::display() {
  auto terminal_half_w = termui::terminal.halfWidth();
  auto terminal_half_h = termui::terminal.halfHeight();

  auto window_start_row = terminal_half_h - (_height / 2);
  auto window_start_col = terminal_half_w - (_width / 2);

  // box
  termui::terminal.MoveCursor(window_start_row, window_start_col);
  termui::Box(_width, _height, termui::Borders::rounded, termui::Styles::faint).render();

  // title
  termui::terminal.MoveCursor(window_start_row + 1, window_start_col + 2);
  termui::Text("KAN I Change Some Settings?", termui::Styles::bold).width(_width - 4).height(1).render();

  auto col_width = (_width - 6) / 2;

  termui::terminal.flush();
}

void SettingsDialog::process_input() {
  auto inp = termui::terminal.read();

  if (inp == termui::key::ESC) {
    state = termui::Interface::State::Exit;
  }
}

void SettingsDialog::show() {

  state = State::Continue;

  do {
    display();
    process_input();

  } while (state == State::Continue);
}

} // namespace tui
} // namespace kan
