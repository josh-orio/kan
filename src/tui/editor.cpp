#include <kan/tui.hpp>

namespace kan {
namespace tui {

void EditWindow::display() {

  auto terminal_half_w = termui::terminal.halfWidth();
  auto terminal_half_h = termui::terminal.halfHeight();

  auto window_start_row = terminal_half_h - (_height / 2) - 1;
  auto window_start_col = terminal_half_w - (_width / 2);

  // box
  termui::terminal.MoveCursor(window_start_row, window_start_col);
  termui::Box(_width, _height, termui::Borders::rounded, termui::Styles::faint).render();

  // control bar

  // input
  termui::terminal.MoveCursor(window_start_row + 1, window_start_col + 1);
  termui::Input(_content).size(_width - 2, _height - 2).valueStyle(termui::Style(termui::Color::Inherit(), termui::Colors::Red)).render();

  termui::terminal.flush();
}

void EditWindow::process_input() {
  auto inp = termui::terminal.read();

  if (inp == termui::key::ESC) {
    state = termui::Interface::State::Exit;
  }
  else if ((std::string{32} <= inp) && (inp <= std::string{126})) {
    std::string &cnt = _content;
    cnt += inp;
  }
  else if (inp == termui::key::DEL) { // remove last char
    std::string &cnt = _content;
    if (cnt.size() > 0) {
      cnt = std::string(cnt.begin(), cnt.end() - 1);
    }
  }
  else if (inp == termui::key::ENTER) {
    std::string &cnt = _content;
    cnt += "\n";
  }
}

EditWindow::EditWindow(std::string content) : _content(content) {}

EditWindow &EditWindow::width(unsigned int n) {
  _width = n;
  return *this;
}

EditWindow &EditWindow::height(unsigned int n) {
  _height = n;
  return *this;
}

std::string EditWindow::show() {
  state = State::Continue;

  do {
    display();
    process_input();

  } while (state == State::Continue);

  return _content;
}

} // namespace tui
} // namespace kan
