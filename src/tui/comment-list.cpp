#include <kan/tui.hpp>

namespace kan {
namespace tui {

CommentList::CommentList(Comments comments) : _comments(comments), _start_line(0), _cursor(0) {}

CommentList &CommentList::size(unsigned int w, unsigned int h) {
  _w = w;
  _h = h;
  return *this;
}

CommentList &CommentList::width(unsigned int w) {
  _w = w;
  return *this;
}

CommentList &CommentList::height(unsigned int h) {
  _h = h;
  return *this;
}

void CommentList::ReloadComments(Comments comments) { _comments = comments; }

void CommentList::CursorUp(unsigned int count) {
  internal_update();
  _cursor     = (_cursor > 0) ? std::max(0, int(_cursor - count)) : 0; // decrement but dont let (cursor < 0)
  _start_line = (_cursor < _start_line) ? _cursor : _start_line;       // don't let cursor go 'above' start line
}

void CommentList::CursorDown(unsigned int count) {
  internal_update();
  _cursor = (_cursor < _comments.size() - count) ? std::min(_comments.size() - 1, static_cast<size_t>(_cursor + count))
                                                 : _comments.size() - 1; // increment but dont let (cursor > elements.size)
  _start_line =
      (_cursor >= _start_line + _visible_comments) ? _cursor - _visible_comments + 1 : _start_line; // don't let cursor go 'below' bottom of visible lines
}

unsigned int CommentList::GetCursor() { return _cursor; }

void CommentList::render() {
  internal_update();

  auto [row, col] = termui::terminal.GetCursorPosition();

  if (_comments.size() == 0) {
    termui::Text("No comments to show.").style(termui::Styles::faint).align(termui::Text::Alignment::Center).width(_w).height(1).render();
  }
  else {
    for (int i = _start_line; i < std::min(_comments.size(), static_cast<size_t>(_start_line + _visible_comments)); i++) {

      if (i + _start_line == _cursor) {
        termui::terminal.MoveCursor(row + (4 * i), col);
        termui::Line(termui::Line::Direction::Vertical, 3).render();
      }

      termui::terminal.MoveCursor(row + (4 * i), col + 2);
      termui::Text(_comments.at(i + _start_line).text).size(_w - 2, 2).render();

      termui::terminal.MoveCursor(row + (4 * i) + 2, col + 2);
      termui::Text(_comments.at(i + _start_line).creator.Name()).style(termui::Styles::bold).size(_w - 2 - 20, 1).render();
      termui::Text(_comments.at(i + _start_line).created.toString()).size(20, 1).align(termui::Text::Alignment::Right).render();
    }
  }

  termui::terminal.MoveCursor(row + _h - 1, col);
  kan::tui::ControlStrip({
                             {"esc", "close"},
                             {"↑↓", "navigate"},
                             {"o/↵", "select"},
                             {"c", "new comment"},
                         })
      .width(_w)
      .render();
}

void CommentList::internal_update() {
  _visible_comments = 0;
  while (true) {
    // calculates lines used by displaying another row of cells
    int space_used = (_cell_height * (_visible_comments + 1)) + (_line_spacing * _visible_comments);

    if (space_used > _h - 2) { // -2 for controlstrip space
      break;
    }
    _visible_comments++;
  }
}

} // namespace tui
} // namespace kan
