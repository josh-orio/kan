#include <kan/tui.hpp>

namespace kan {
namespace tui {

void HelpDialog::display() {
  auto terminal_half_w = termui::terminal.halfWidth();
  auto terminal_half_h = termui::terminal.halfHeight();

  auto window_start_row = terminal_half_h - (_height / 2);
  auto window_start_col = terminal_half_w - (_width / 2);

  // box
  termui::terminal.MoveCursor(window_start_row, window_start_col);
  termui::Box(_width, _height, termui::Borders::rounded, termui::Styles::faint).render();

  // title
  termui::terminal.MoveCursor(window_start_row + 1, window_start_col + 2);
  termui::Text("KAN I Help You?", termui::Styles::bold).width(_width - 4).height(1).render();

  auto col_width = (_width - 6) / 2;

  termui::terminal.MoveCursor(window_start_row + 4, window_start_col + 3);
  termui::terminal.write("tab");
  termui::terminal.write("  ");
  termui::terminal.StyleStack(termui::Styles::faint);
  termui::terminal.write("section");
  termui::terminal.StylePop();

  termui::terminal.MoveCursor(window_start_row + 6, window_start_col + 3);
  termui::terminal.write("enter");
  termui::terminal.write("  ");
  termui::terminal.StyleStack(termui::Styles::faint);
  termui::terminal.write("select");
  termui::terminal.StylePop();

  termui::terminal.MoveCursor(window_start_row + 8, window_start_col + 3);
  termui::terminal.write("←↑↓→");
  termui::terminal.write("  ");
  termui::terminal.StyleStack(termui::Styles::faint);
  termui::terminal.write("navigate");
  termui::terminal.StylePop();

  termui::terminal.MoveCursor(window_start_row + 10, window_start_col + 3);
  termui::terminal.write("esc");
  termui::terminal.write("  ");
  termui::terminal.StyleStack(termui::Styles::faint);
  termui::terminal.write("close");
  termui::terminal.StylePop();

  termui::terminal.MoveCursor(window_start_row + 12, window_start_col + 3);
  termui::terminal.write("q");
  termui::terminal.write("  ");
  termui::terminal.StyleStack(termui::Styles::faint);
  termui::terminal.write("quit");
  termui::terminal.StylePop();

  termui::terminal.flush();
}

void HelpDialog::process_input() {
  auto inp = termui::terminal.read();

  if (inp == termui::key::ESC) {
    state = termui::Interface::State::Exit;
  }
}

void HelpDialog::show() {

  state = State::Continue;

  do {
    display();
    process_input();

  } while (state == State::Continue);
}

} // namespace tui
} // namespace kan
