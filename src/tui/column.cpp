#include <kan/tui.hpp>

namespace kan {
namespace tui {

Column::Column(kan::Column &column) : _column(&column), _cursor(0), _start_line(0) {}

Column &Column::filter(std::vector<unsigned int> rows) {
  if (rows != _rows) { // the result set changed, start from the top again
    _cursor     = 0;
    _start_line = 0;
  }

  _rows = rows;
  return *this;
}

Column &Column::width(uint n) {
  _width = n;
  return *this;
}
Column &Column::height(uint n) {
  _height = n;
  return *this;
}

Column &Column::focus() {
  _focussed = true;
  return *this;
}
Column &Column::blur() {
  _focussed = false;
  return *this;
}

Column &Column::cursorUp() {
  _cursor > 0 ? _cursor-- : _cursor;
  _cursor < _start_line ? _start_line = _cursor : _start_line;
  return *this;
}

Column &Column::cursorDown() {

  _cursor + 1 < _rows.size() ? _cursor++ : _cursor;
  _cursor >= _start_line + _visible_rows ? _start_line = _cursor - _visible_rows + 1 : _start_line;
  return *this;
}

Column &Column::setCursor(unsigned int row) {
  _cursor = std::min<unsigned int>(row, _rows.size());
  return *this;
}

Column &Column::gotoTop() {
  _cursor = 0;
  return *this;
}

Column &Column::gotoBottom() {
  _cursor = _rows.empty() ? 0 : _rows.size() - 1;
  return *this;
}

void Column::render() {
  if (_width < 2 || _height < 2) {
    return;
  }

  internal_update();

  // clear space
  termui::Text("").height(_height).width(_width).render();
  termui::terminal.CursorUp(_height - 1).CursorLeft(_width);

  // column title
  termui::PaddedText(_column->title, _column->header, termui::PaddingStyle::label).width(_width).height(1).render();
  termui::terminal.CursorDown(2).CursorLeft(_width);

  int line_height  = 2;
  int line_spacing = 2;

  if (_focussed && _rows.empty()) {
    auto row_style = board.styleMap.at("highlight");
    termui::Text("").width(_width).height(2).style(row_style).render();
  }
  else {
    for (int i = _start_line; i < std::min(_rows.size(), static_cast<size_t>(_start_line + _visible_rows)); i++) {
      JobCell(_column->Jobs()[_rows[i]]).width(_width).setFocus(_focussed && i == _cursor).render();
      // give style here
      termui::terminal.CursorDown(3).CursorLeft(_width);
    }
  }
}

void Column::internal_update() {
  _visible_rows = 0;
  while (true) {
    // calculates lines used by displaying another row of cells
    unsigned int space_used = (2 * (_visible_rows + 1)) + (2 * _visible_rows);

    if (space_used > _height - 2) { // -2 for title
      break;
    }
    _visible_rows++;
  }

  if ( _cursor < _start_line) {
    _start_line=_cursor;
  }
  else if (_cursor >= _start_line + _visible_rows) {
    _start_line = _cursor - _visible_rows; // maybe -1
  }
}

} // namespace tui
} // namespace kan
