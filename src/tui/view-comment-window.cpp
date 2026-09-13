#include <kan/tui.hpp>

namespace kan {
namespace tui {

ViewCommentWindow::ViewCommentWindow(kan::Comment comment) : _comment(comment), _pager(_comment.text) {}

void ViewCommentWindow::display() {
  auto terminal_half_w = termui::terminal.halfWidth();
  auto terminal_half_h = termui::terminal.halfHeight();

  auto window_start_row = terminal_half_h - (_height / 2);
  auto window_start_col = terminal_half_w - (_width / 2);

  // auto tab = _tab_bar.GetTab();

  // box
  termui::terminal.MoveCursor(window_start_row, window_start_col);
  termui::Box(_width, _height, termui::Borders::rounded, termui::Styles::faint).render();

  // user details
  termui::terminal.MoveCursor(window_start_row + 1, window_start_col + 2);
  termui::Text(_comment.creator.toString()).style(termui::Styles::bold).width(_width - 4).render();

  // timestamp
  termui::terminal.MoveCursor(window_start_row + 2, window_start_col + 2);
  termui::Text(_comment.created.toString()).width(_width - 4).render();

  // content
  termui::terminal.MoveCursor(window_start_row + 4, window_start_col + 2);
  // termui::Text(_comment.text).width(_width - 4).height(_height - 7).render();
  // termui::Pager(_comment.text).width(_width - 4).height(_height - 7).style(termui::Style(termui::Color::Inherit(), termui::Colors::Red)).render();
  _pager.width(_width - 4).height(_height - 7).style(termui::Style(termui::Color::Inherit(), termui::Colors::Red)).render();

  // control strip
  termui::terminal.MoveCursor(window_start_row + _height - 2, window_start_col + 2);
  kan::tui::ControlStrip({
                             {"esc", "close"},
                             {"↑↓", "navigate"},
                             {"d", "delete comment"},
                         })
      .width(_width - 4)
      .render();

  termui::terminal.flush();
}

void ViewCommentWindow::process_input() {
  auto inp = termui::terminal.read();

  if (inp == termui::key::ESC) {
    state = termui::Interface::State::Exit;
  }
  else if (inp == termui::key::U_ARROW) {
    _pager.cursor_up();
  }
  else if (inp == termui::key::D_ARROW) {
    _pager.cursor_down();
  }
  else if (inp == "d") {
    auto confirm = DialogBox("Confirm delete", "Are you sure you want to delete this comment?", "Delete", "Cancel").show();

    if (confirm) {
      state = termui::Interface::State::Delete;
    }
  }
}

termui::Interface::State ViewCommentWindow::show() {
  state = State::Continue;

  do {
    display();
    process_input();

  } while (state == State::Continue);

  return state;
}

} // namespace tui
} // namespace kan
