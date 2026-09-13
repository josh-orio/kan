#include <kan/tui.hpp>

namespace kan {
namespace tui {

NewCommentWindow::NewCommentWindow() {}

void NewCommentWindow::display() {
  auto terminal_half_w = termui::terminal.halfWidth();
  auto terminal_half_h = termui::terminal.halfHeight();

  auto window_start_row = terminal_half_h - (_height / 2);
  auto window_start_col = terminal_half_w - (_width / 2);

  // auto tab = _tab_bar.GetTab();

  // box
  termui::terminal.MoveCursor(window_start_row, window_start_col);
  termui::Box(_width, _height, termui::Borders::rounded, termui::Styles::faint).render();

  // title
  termui::terminal.MoveCursor(window_start_row + 1, window_start_col + 2);
  termui::Text("New Comment", termui::Styles::bold).width(_width - 4).height(1).render();

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

  termui::Button save = termui::Button("Save", termui::Style(termui::Color::Inherit(), termui::Colors::Red, {termui::SGR::Bold}), termui::Style()).width(6);

  if (_selected_field == Focus::Save) {
    save.focus();
  }
  else {
    save.blur();
  }

  save.render();

  // input region

  termui::terminal.MoveCursor(window_start_row + 3, window_start_col + 2);
  // termui::Text("New Comment", termui::Styles::bold).width(_width - 4).height(1).render();
  termui::Input(_comment, "Type your comment...").size(_width - 4, 3).render();

  if (_selected_field == Focus::Text) {
    termui::terminal.ShowCursor();
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
      std::string &cmt = _comment;
      cmt += inp;
    }
  }
  else if (inp == termui::key::DEL) { // remove last char
    if (_selected_field == Focus::Text) {

      std::string &cmt = _comment;
      if (cmt.size() > 0) {
        cmt = std::string(cmt.begin(), cmt.end() - 1);
      }
    }
  }
  else if (inp == termui::key::ENTER) {
    if (_selected_field == Focus::Text) {
      std::string &cmt = _comment;
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
    Comment comment;
    comment.text    = _comment;
    comment.creator = User().getFromSystem();
    comment.created = Timestamp().Now();
    comment.uuid    = UUIDv7::generate().to_string();

    return {true, comment};
  }
  else /* Exit */ {
    return {false, Comment()};
  }
}

} // namespace tui
} // namespace kan
