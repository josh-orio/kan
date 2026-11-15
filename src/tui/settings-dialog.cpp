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

  // f toggles finishing jobs that get moved into the last column
  auto last = board.Statuses().empty() ? std::string("the last column") : board.Statuses().back();

  termui::terminal.MoveCursor(window_start_row + 3, window_start_col + 4);
  termui::Text(board.finish_on_last_column ? "[x]" : "[ ]", termui::Styles::bold).width(3).height(1).render();
  termui::Text(std::format(" finish jobs moved into {}", last)).width(_width - 4 - 4 - 3).height(1).render();

  termui::terminal.MoveCursor(window_start_row + _height - 2, window_start_col + 2);
  kan::tui::ControlStrip({
                             {"esc", "close"},
                             {"f", "toggle"},
                         })
      .width(_width - 4)
      .render();

  termui::terminal.flush();
}

void SettingsDialog::process_input() {
  auto inp = termui::terminal.read();

  if (inp == termui::key::ESC || inp == "q") {
    state = termui::Interface::State::Exit;
  }
  else if (inp == "f") {
    board.finish_on_last_column = !board.finish_on_last_column;
    board.saveBoard();
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
