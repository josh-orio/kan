#include <kan/tui.hpp>

namespace kan {
namespace tui {

Column::Column(kan::Column &column) : _column(&column), _cursor(0), _start_line(0) {}

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

  _cursor + 1 < _column->jobs.size() ? _cursor++ : _cursor;
  _cursor >= _start_line + _visible_rows ? _start_line = _cursor - _visible_rows + 1 : _start_line;
  return *this;
}

Column &Column::setCursor(unsigned int row) {
  _cursor = std::min<unsigned int>(row, _column->jobs.size());
  return *this;
}

Column &Column::gotoTop() {
  _cursor = 0;
  return *this;
}

Column &Column::gotoBottom() {
  _cursor = _column->jobs.size() - 1;
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
  termui::PaddedText(_column->title, termui::Styles::bold, termui::PaddingStyle::label).width(_width).height(1).render();
  termui::terminal.CursorDown(2).CursorLeft(_width);

  int line_height  = 2;
  int line_spacing = 2;

  if (_focussed && _column->Jobs().size() == 0) {
    auto row_style = board.styleMap.at("highlight");
    termui::Text("").width(_width).height(2).style(row_style).render();
  }
  else {
    for (int i = _start_line; i < std::min(_column->Jobs().size(), static_cast<size_t>(_start_line + _visible_rows)); i++) {
      JobCell(_column->Jobs()[i]).width(_width).setFocus(_focussed && i == _cursor).render();
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
