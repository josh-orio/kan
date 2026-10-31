#include <kan/tui.hpp>

namespace kan {
namespace tui {

NewCommentWindow::NewCommentWindow() {}

void NewCommentWindow::display() {
  auto terminal_half_w = termui::terminal.halfWidth();
  auto terminal_half_h = termui::terminal.halfHeight();

  auto window_start_row = terminal_half_h - (_height / 2);
  auto window_start_col = terminal_half_w - (_width / 2);

  // box
  termui::terminal.MoveCursor(window_start_row, window_start_col);
  termui::Box(_width, _height, termui::Borders::rounded, termui::Styles::faint).render();

  // title
  termui::terminal.MoveCursor(window_start_row + 1, window_start_col + 2);
  termui::Text("New Comment", termui::Styles::bold).width(_width - 4).height(1).render();

  // input region
  termui::terminal.MoveCursor(window_start_row + 3, window_start_col + 2);
  termui::Input(_comment.text, "Type your comment...").size(_width - 4, 3).focussed(_selected_field == Focus::Text).render();

  auto [row,col] = termui::terminal.GetFutureCursorPosition();

  // control bar
  termui::terminal.MoveCursor(window_start_row + 7, window_start_col + 2);
  if (_selected_field == Focus::Text) {
    kan::tui::ControlStrip({
                               {"esc", "close"},
                               {"↑↓", "navigate"},
                           })
        .width(_width - 11)
        .render();
  }
  else if (_selected_field == Focus::Save) {
    kan::tui::ControlStrip({
                               {"esc", "close"},
                               {"↑↓", "navigate"},
                               {"↵", "save"},
                           })
        .width(_width - 11)
        .render();
  }

  // save button
  termui::Button("Save").setFocus(_selected_field == Focus::Save).width(6).render();

  if (_selected_field == Focus::Text) {
    termui::terminal.ShowCursor();
    termui::terminal.MoveCursor(row, col);
  }
  else {
    termui::terminal.HideCursor();
  }

  termui::terminal.flush();
}

void NewCommentWindow::process_input() {
  auto inp = termui::terminal.read();

  if (inp == termui::key::ESC) {
    state = termui::Interface::State::Exit;
  }
  else if (inp == termui::key::U_ARROW) {
    _selected_field = Focus::Text;
  }
  else if (inp == termui::key::D_ARROW) {
    _selected_field = Focus::Save;
  }
  else if ((std::string{32} <= inp) && (inp <= std::string{126})) { // accept basically all chars
    if (_selected_field == Focus::Text) {
      std::string &cmt = _comment.text;
      cmt += inp;
    }
  }
  else if (inp == termui::key::DEL) { // remove last char
    if (_selected_field == Focus::Text) {

      std::string &cmt = _comment.text;
      if (cmt.size() > 0) {
        cmt = std::string(cmt.begin(), cmt.end() - 1);
      }
    }
  }
  else if (inp == termui::key::ENTER) {
    if (_selected_field == Focus::Text) {
      std::string &cmt = _comment.text;
      cmt += "\n";
    }
    else if (_selected_field == Focus::Save) {
      state = termui::Interface::State::Submit;
    }
  }
}

std::pair<bool, kan::Comment> NewCommentWindow::show() {
  state = State::Continue;

  do {
    display();
    process_input();

  } while (state == State::Continue);

  termui::terminal.HideCursor();

  if (state == termui::Interface::State::Submit) {
    _comment.creator = User().getFromSystem();
    _comment.created = Timestamp().Now();
    // _comment.uuid    = UUID::generate().to_string();

    return {true, _comment};
  }
  else /* Exit */ {
    return {false, kan::Comment()};
  }
}

} // namespace tui
} // namespace kan
