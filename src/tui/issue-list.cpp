#include <kan/tui.hpp>

namespace kan {
namespace tui {

IssueList::IssueList(Issues issues) : _issues(issues), _start_line(0), _cursor(0) {}

IssueList &IssueList::size(unsigned int w, unsigned int h) {
  _w = w;
  _h = h;
  return *this;
}

IssueList &IssueList::width(unsigned int w) {
  _w = w;
  return *this;
}

IssueList &IssueList::height(unsigned int h) {
  _h = h;
  return *this;
}

// void IssueList::ReloadIssues() { }

void IssueList::CursorUp(unsigned int count) {
  internal_update();
  _cursor = (_cursor > 0) ? std::max(0, int(_cursor - count)) : 0; // decrement but dont let (cursor < 0)
  _start_line = (_cursor < _start_line) ? _cursor : _start_line;   // don't let cursor go 'above' start line
}

void IssueList::CursorDown(unsigned int count) {
  internal_update();
  _cursor = (_cursor < _issues.size() - count) ? std::min(_issues.size() - 1, static_cast<size_t>(_cursor + count))
                                               : _issues.size() - 1;                                      // increment but dont let (cursor > elements.size)
  _start_line = (_cursor >= _start_line + _visible_issues) ? _cursor - _visible_issues + 1 : _start_line; // don't let cursor go 'below' bottom of visible lines
}

unsigned int IssueList::GetCursor() { return _cursor; }

void IssueList::render() {
  internal_update();

  auto [row, col] = termui::terminal.GetCursorPosition();

  if (_issues.size() == 0) {
    termui::Text("No issues to show.").style(termui::Styles::faint).align(termui::Text::Alignment::Center).width(_w).height(1).render();

  } else {

    for (int i = _start_line; i < std::min(_issues.size(), static_cast<size_t>(_start_line + _visible_issues)); i++) {

      if (i + _start_line == _cursor) {
        termui::terminal.MoveCursor(row + (4 * i), col);
        termui::Line(termui::Line::Direction::Vertical, 3).render();
      }

      termui::terminal.MoveCursor(row + (4 * i), col + 2);
      // termui::Text(_issues.at(i + _start_line).text).size(_w - 2, 2).render();

      // termui::terminal.CursorLeft(_w).CursorDown(_line_spacing + 1);

      termui::terminal.MoveCursor(row + (4 * i) + 2, col + 2);
      termui::Text(_issues.at(i + _start_line).creator.Name()).style(termui::Styles::bold).size(_w - 2 - 20, 1).render();
      termui::Text(_issues.at(i + _start_line).created.toString()).size(20, 1).align(termui::Text::Alignment::Right).render();
    }
  }
  termui::terminal.MoveCursor(row + _h - 1, col);

  kan::tui::ControlStrip({
                             {"esc", "close"},
                             {"↑↓", "navigate"},
                             {"o/↵", "select"},
                             {"i", "new issue"},
                         })
      .width(_w)
      .render();
}

void IssueList::internal_update() {
  _visible_issues = 0;
  while (true) {
    // calculates lines used by displaying another row
    int space_used = (3 * (_visible_issues + 1)) + (_line_spacing * _visible_issues); // 3* because cell is 3 lines high

    if (space_used > _h - 2) { // -2 for controlstrip space
      break;
    }
    _visible_issues++;
  }
}

} // namespace tui
} // namespace kan
