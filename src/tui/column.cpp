#include <kan/tui.hpp>

namespace kan {
namespace tui {

Column::Column(kan::Column column) : _column(column), _active_row(0) {}

Column &Column::width(uint n) {
  _width = n;
  return *this;
}
Column &Column::height(uint n) {
  _height = n;
  return *this;
}

Column &Column::focus(uint row) {
  _focussed   = true;
  _active_row = std::min<int>(row, _column.Jobs().size());
  return *this;
}
Column &Column::blur() {
  _focussed = false;
  return *this;
}

void Column::render() {
  if (_width < 2 || _height < 2) {
    return;
  }

  auto [row, col] = termui::terminal.GetCursorPosition();

  // box
  termui::Box(_width, _height, termui::Borders::rounded, termui::Styles::faint).render();

  // column title
  termui::terminal.MoveCursor(row + 1, col + 2);
  termui::Text(_column.title, termui::Styles::bold, _width - 4, 1).render();

  int line_height  = 2;
  int line_spacing = 2;

  int h             = _height;
  int visible_lines = _height <= 0 ? 0 : [h]() {
    int overhead     = 2 + 1;
    int line_spacing = 2;
    int line_height  = 2;

    return (h - overhead) / (line_height + line_spacing);
  }();

  int _start_line = 0;

  if (_focussed && _column.Jobs().size() == 0) {
    termui::terminal.MoveCursor(row + 3, col + 1);
    termui::Text("").width(_width - 2).height(2).style(termui::Style({termui::SGR::ReverseVideo})).render();
  }
  else {
    for (int i = _start_line; i < std::min(_column.Jobs().size(), static_cast<size_t>(_start_line + visible_lines)); i++) {
      termui::terminal.MoveCursor(row + 3 + (4 * i), col + 1);
      JobCell(_column.Jobs()[i]).width(_width - 2).setFocus(_focussed && i == _active_row).render();
    }
  }
}

} // namespace tui
} // namespace kan
