#include <kan/tui.hpp>

namespace kan {
namespace tui {

CommitWindow::CommitWindow(kan::Commit commit) : _commit(commit) {
  // prefer git's copy, fall back to what the job has stored if the commit is gone (rebased, not fetched...)
  try {
    _commit = git::readCommit(commit.hash);
    _files  = git::changedFiles(commit.hash);
  }
  catch (const std::exception &e) {
    _error = "Couldn't read this commit from git - it may have been rebased away or not fetched yet.";
  }
}

static termui::Color statusColor(char status) {
  switch (status) {
  case 'A':
    return termui::Colors::Green;
  case 'D':
    return termui::Colors::Red;
  case 'R':
  case 'C':
    return termui::Colors::Blue;
  default:
    return termui::Colors::Yellow;
  }
}

void CommitWindow::display() {
  _width  = termui::terminal.width();
  _height = termui::terminal.height();

  termui::terminal.ClearScreen();
  termui::terminal.HideCursor();

  auto content_w = _width - 4;

  auto newline = _commit.message.find('\n');
  auto title   = _commit.message.substr(0, newline);
  auto body    = newline == std::string::npos ? std::string() : _commit.message.substr(newline + 1);

  body.erase(0, body.find_first_not_of('\n'));

  // title
  termui::terminal.MoveCursor(1, 2);
  termui::TextRow(content_w).add(termui::Text(title, termui::Styles::bold)).add(termui::Text(_commit.hash.substr(0, 7), termui::Styles::faint), 7).render();

  // author + date
  termui::terminal.MoveCursor(3, 2);
  termui::TextRow(content_w)
      .add(termui::Text(_commit.creator.toString()))
      .add(termui::Text(_commit.created.toString()).align(termui::Text::Alignment::Right), 20)
      .render();

  unsigned int r = 5;

  // full hash
  termui::terminal.MoveCursor(r, 2);
  termui::Text(_commit.hash, termui::Styles::faint).width(content_w).height(1).render();
  r += 2;

  // body - capped so a long message doesn't push the file list off screen
  if (!body.empty()) {
    auto max_h  = std::max<unsigned int>(1, _height / 3);
    auto [w, h] = termui::Text(body).width(content_w).size();
    auto body_h = std::min<unsigned int>(h, max_h);

    termui::terminal.MoveCursor(r, 2);
    termui::Text(body).width(content_w).height(body_h).render();
    r += body_h + 1;
  }

  if (!_error.empty()) {
    termui::terminal.MoveCursor(r, 2);
    termui::Text(_error, termui::Styles::faint).width(content_w).render();
  }
  else {
    int additions = 0, deletions = 0;
    for (auto &f : _files) {
      additions += f.additions;
      deletions += f.deletions;
    }

    // files header
    termui::terminal.MoveCursor(r, 2);
    termui::TextRow(content_w)
        .add(termui::Text("Files changed", termui::Styles::bold))
        .add(termui::Text(std::format("{} {}", _files.size(), _files.size() == 1 ? "file" : "files"), termui::Styles::faint))
        .add(termui::Text(std::format("+{}", additions), termui::Style(termui::Colors::Green, termui::Color::Inherit())))
        .add(termui::Text(std::format("-{}", deletions), termui::Style(termui::Colors::Red, termui::Color::Inherit())))
        .render();
    r += 2;

    // file list
    _visible_rows = _height > r + 2 ? _height - r - 2 : 0;

    if (_cursor < _start_line) {
      _start_line = _cursor;
    }
    if (_visible_rows > 0 && _cursor >= _start_line + _visible_rows) {
      _start_line = _cursor - _visible_rows + 1;
    }

    if (_files.empty()) {
      termui::terminal.MoveCursor(r, 2);
      termui::Text("No file changes to show.", termui::Styles::faint).width(content_w).render();
    }

    for (unsigned int i = _start_line; i < std::min<size_t>(_files.size(), _start_line + _visible_rows); i++) {
      auto &f     = _files.at(i);
      bool  focus = i == _cursor;
      auto  style = focus ? board.styleMap.at("highlight") : termui::Style();
      auto  row   = r + (i - _start_line);

      if (focus) {
        termui::terminal.MoveCursor(row, 0);
        termui::Line(termui::Line::Direction::Vertical, 1).render();
      }

      auto path  = f.old_path.empty() ? f.path : std::format("{} → {}", f.old_path, f.path);
      auto stats = f.binary ? std::string("binary") : std::format("+{} -{}", f.additions, f.deletions);

      termui::terminal.MoveCursor(row, 2);
      termui::TextRow(content_w)
          .paddingStyle(style)
          .add(termui::Text(std::string(1, f.status), termui::Style(statusColor(f.status), style.backgroundColor, {termui::SGR::Bold})), 1)
          .add(termui::Text(path, style))
          .add(termui::Text(stats, termui::Style(style.foregroundColor, style.backgroundColor, {termui::SGR::Faint})).align(termui::Text::Alignment::Right), 14)
          .render();
    }
  }

  // control strip
  termui::terminal.MoveCursor(_height - 1, 2);
  kan::tui::ControlStrip({
                             {"esc", "close"},
                             {"↑↓", "navigate"},
                             {"t", "top"},
                             {"b", "bottom"},
                         })
      .width(content_w)
      .render();

  termui::terminal.flush();
}

void CommitWindow::process_input() {
  auto inp = termui::terminal.read();

  if (inp == termui::key::ESC || inp == "q") {
    state = State::Exit;
  }
  else if (inp == termui::key::U_ARROW) {
    _cursor = _cursor > 0 ? _cursor - 1 : 0;
  }
  else if (inp == termui::key::D_ARROW) {
    _cursor = _cursor + 1 < _files.size() ? _cursor + 1 : _cursor;
  }
  else if (inp == "t") {
    _cursor = 0;
  }
  else if (inp == "b") {
    _cursor = _files.empty() ? 0 : _files.size() - 1;
  }
}

void CommitWindow::show() {
  termui::terminal.ClearScreen();
  termui::terminal.ClearScrollback();

  state = State::Continue;

  do {
    display();
    process_input();

  } while (state == State::Continue);

  termui::terminal.ClearScreen();
}

} // namespace tui
} // namespace kan
