#include <kan/tui.hpp>

namespace kan {
namespace tui {

CommitList::CommitList(Commits commits) { Reload(commits); }

CommitList &CommitList::size(unsigned int w, unsigned int h) {
  _w = w;
  _h = h;
  return *this;
}

void CommitList::Reload(Commits commits) {
  _commits = std::vector<kan::Commit>(commits.begin(), commits.end());

  // newest first
  std::sort(_commits.begin(), _commits.end(), [](const kan::Commit &a, const kan::Commit &b) { return a.created.time > b.created.time; });

  _cursor = std::min<unsigned int>(_cursor, _commits.empty() ? 0 : _commits.size() - 1);
}

void CommitList::CursorUp(unsigned int count) {
  internal_update();
  _cursor     = (_cursor > count) ? _cursor - count : 0;
  _start_line = (_cursor < _start_line) ? _cursor : _start_line;
}

void CommitList::CursorDown(unsigned int count) {
  if (_commits.empty()) {
    return;
  }

  internal_update();
  _cursor     = std::min<unsigned int>(_commits.size() - 1, _cursor + count);
  _start_line = (_cursor >= _start_line + _visible_rows) ? _cursor - _visible_rows + 1 : _start_line;
}

void CommitList::GotoTop() {
  _cursor     = 0;
  _start_line = 0;
}

void CommitList::GotoBottom() {
  if (_commits.empty()) {
    return;
  }

  internal_update();
  _cursor     = _commits.size() - 1;
  _start_line = (_cursor >= _visible_rows) ? _cursor - _visible_rows + 1 : 0;
}

bool CommitList::Empty() { return _commits.empty(); }

kan::Commit CommitList::Selected() { return _commits.at(_cursor); }

unsigned int CommitList::GetCursor() { return _cursor; }

void CommitList::render() {
  internal_update();

  auto [row, col] = termui::terminal.GetFutureCursorPosition();

  if (_commits.empty()) {
    termui::Text("No commits reference this job yet.").style(termui::Styles::faint).align(termui::Text::Alignment::Center).width(_w).height(1).render();
  }
  else {
    for (unsigned int i = _start_line; i < std::min<size_t>(_commits.size(), _start_line + _visible_rows); i++) {
      auto &commit = _commits.at(i);
      auto  r      = row + (2 * (i - _start_line)); // one blank row between commits
      bool  focus  = i == _cursor;

      auto style       = focus ? board.styleMap.at("highlight") : termui::Style();
      auto bold_style  = termui::Style(style.foregroundColor, style.backgroundColor, {termui::SGR::Bold});
      auto faint_style = termui::Style(style.foregroundColor, style.backgroundColor, {termui::SGR::Faint});

      // title is the first line, the rest of the message is flattened onto the same row
      auto newline = commit.message.find('\n');
      auto title   = commit.message.substr(0, newline);
      auto rest    = newline == std::string::npos ? std::string() : commit.message.substr(newline + 1);

      rest.erase(0, rest.find_first_not_of(" \n"));
      std::replace(rest.begin(), rest.end(), '\n', ' ');

      if (focus) {
        termui::terminal.MoveCursor(r, col);
        termui::Line(termui::Line::Direction::Vertical, 1).render();
      }

      // hash + date are pinned together on the right, the message takes whatever is left
      auto date    = commit.created.toString();
      auto right_w = 7 + 1 + date.size();
      auto left_w  = (_w - 2) > right_w + 1 ? (_w - 2) - right_w - 1 : 0;

      termui::terminal.MoveCursor(r, col + 2);
      termui::TextRow(left_w)
          .paddingStyle(style)
          .add(termui::Text(title, bold_style))
          .add(termui::Text(rest, faint_style))
          .render();

      termui::Text(" ", style).width(1).height(1).render();

      termui::TextRow(right_w)
          .paddingStyle(style)
          .add(termui::Text(commit.hash.substr(0, 7), faint_style), 7)
          .add(termui::Text(date, style))
          .render();
    }
  }

  termui::terminal.MoveCursor(row + _h - 1, col);
  kan::tui::ControlStrip({
                             {"esc", "close"},
                             {"↑↓", "navigate"},
                             {"t", "top"},
                             {"b", "bottom"},
                             {"↵", "view commit"},
                         })
      .width(_w)
      .render();
}

void CommitList::internal_update() {
  _visible_rows = _h > 2 ? (_h - 2 + 1) / 2 : 0; // -2 for controlstrip space, each row is 1 line + 1 gap

  if (_cursor < _start_line) {
    _start_line = _cursor;
  }
  if (_visible_rows > 0 && _cursor >= _start_line + _visible_rows) {
    _start_line = _cursor - _visible_rows + 1;
  }
}

} // namespace tui
} // namespace kan
