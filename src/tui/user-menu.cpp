#include <kan/tui.hpp>

namespace kan {
namespace tui {

UserMenu::UserMenu() : _users(board.contributors), _filtered(_users), _cursor(0), _start_line(0) {}

void UserMenu::apply_filter() {
  auto lower = [](std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });
    return str;
  };

  auto query = lower(_query);

  _filtered.clear();
  for (auto &user : _users) {
    if (lower(user.name).find(query) != std::string::npos) {
      _filtered.push_back(user);
    }
  }

  _cursor     = 0;
  _start_line = 0;
}

void UserMenu::CursorUp(unsigned int count) {
  internal_update();
  _cursor     = (_cursor > 0) ? std::max(0, int(_cursor - count)) : 0; // decrement but dont let (cursor < 0)
  _start_line = (_cursor < _start_line) ? _cursor : _start_line;       // don't let cursor go 'above' start line
}

void UserMenu::CursorDown(unsigned int count) {
  internal_update();
  if (_filtered.empty()) {
    return;
  }
  _cursor     = (_cursor < _filtered.size() - count) ? std::min(_filtered.size() - 1, static_cast<size_t>(_cursor + count))
                                                     : _filtered.size() - 1;                            // increment but dont let (cursor > elements.size)
  _start_line = (_cursor >= _start_line + _visible_lines) ? _cursor - _visible_lines + 1 : _start_line; // don't let cursor go 'below' bottom of visible lines
}

unsigned int UserMenu::getCursor() { return _cursor; }

UserMenu &UserMenu::setCursor(unsigned int n) {
  _cursor = n;
  return *this;
}

void UserMenu::display() {
  internal_update();

  auto terminal_half_w = termui::terminal.halfWidth();
  auto terminal_half_h = termui::terminal.halfHeight();

  auto window_start_row = terminal_half_h - (_height / 2);
  auto window_start_col = terminal_half_w - (_width / 2);

  // box
  termui::terminal.MoveCursor(window_start_row, window_start_col);
  termui::Box(_width, _height, termui::Borders::rounded, termui::Styles::faint).render();

  // title header
  termui::terminal.MoveCursor(window_start_row + 1, window_start_col + 2);
  termui::Text(_title, termui::Styles::bold).width(_width - 4).height(1).render();

  // filter input
  termui::terminal.MoveCursor(window_start_row + 3, window_start_col + 2);
  termui::Text(std::format("> {}", _query)).size(_width - 4, 1).render();

  if (_filtered.empty()) {
    termui::terminal.MoveCursor(window_start_row + 5, window_start_col + 4);
    termui::Text(_users.empty() ? "no contributors yet" : "no matches", termui::Styles::faint).size(_width - 6, 1).render();
  }

  // list items
  for (size_t i = _start_line; i < std::min(_filtered.size(), static_cast<size_t>(_start_line + _visible_lines)); i++) {
    int line_pos = window_start_row + 5 + (i - _start_line) * (_line_height + _line_spacing);

    if (i == _cursor) {
      termui::terminal.MoveCursor(line_pos, window_start_col + 2);
      termui::terminal.write("·");
    }

    termui::terminal.MoveCursor(line_pos, window_start_col + 4);
    termui::Text(_filtered.at(i).Name()).size(_width - 6, 1).render();
  }

  // control strip
  termui::terminal.MoveCursor(window_start_row + _height - 2, window_start_col + 2);
  kan::tui::ControlStrip({
                             {"esc", "close"},
                             {"↑↓", "navigate"},
                             {"↵", "select"},
                             {"type", "filter"},
                         })
      .width(_width - 4)
      .render();

  termui::terminal.flush();
}

void UserMenu::process_input() {
  auto inp = termui::terminal.read();

  if (inp == termui::key::ESC) {
    state = termui::Interface::State::Exit;
  }
  else if (inp == termui::key::U_ARROW) {
    CursorUp();
  }
  else if (inp == termui::key::D_ARROW) {
    CursorDown();
  }
  else if (inp == termui::key::ENTER) {
    if (!_filtered.empty()) {
      state = termui::Interface::State::Submit;
    }
  }
  else if (inp == termui::key::DEL) {
    if (!_query.empty()) {
      _query.pop_back();
      apply_filter();
    }
  }
  else if (inp.size() == 1 && std::isprint(static_cast<unsigned char>(inp[0]))) {
    _query += inp;
    apply_filter();
  }
}

UserMenu &UserMenu::setCursor(const User &user) {
  auto it = std::find(_filtered.begin(), _filtered.end(), user);
  if (it != _filtered.end()) {
    _cursor = it - _filtered.begin();
  }
  return *this;
}

std::pair<termui::Interface::State, User> UserMenu::show() {
  state = State::Continue;

  // make sure the cursor starts inside the visible window
  internal_update();
  _start_line = (_cursor >= _visible_lines) ? _cursor - _visible_lines + 1 : 0;

  do {
    display();
    process_input();

  } while (state == State::Continue);

  if (state != State::Submit) {
    return {state, User()};
  }

  return {state, _filtered.at(_cursor)};
}

void UserMenu::internal_update() {
  _visible_lines = 0;
  while (true) {
    // calculates lines used by displaying another row of cells
    int space_used = (_line_height * (_visible_lines + 1)) + (_line_spacing * _visible_lines);

    if (space_used > _height - 8) { // -8 for other stuff
      break;
    }
    _visible_lines++;
  }
}

} // namespace tui
} // namespace kan