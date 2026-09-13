#include <kan/tui.hpp>

namespace kan {
namespace tui {

// Column::Column(termui::string title, std::vector<Job> jobs) : _title(title), _jobs(jobs) {}
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

  termui::Box(_width, _height, termui::Borders::rounded, termui::Styles::faint).render();
  termui::terminal.CursorUp(_height - 2).CursorLeft(_width - 1);

  // termui::Text(_column._title + (_focussed ? " focussed " + std::format("{}", _active_row) : ""), termui::Styles::bold, _width - 2, 1).render();
  termui::Text(_column.Title(), termui::Styles::bold, _width - 2, 1).render();
  termui::terminal.CursorLeft(1).CursorUp(1);

  int line_height  = 2;
  int line_spacing = 2;

  int h             = _height;
  int visible_lines = _height <= 0 ? 0 : [h]() {
    int overhead     = 2 + 1;
    int line_spacing = 2;
    int line_height  = 2;

    return (h - overhead) / (line_height + line_spacing);
  }();

  int down_set = _height - 2;

  auto cell_style = termui::Styles::none;

  for (int i = 0; i < std::min((int)_column.Jobs().size(), visible_lines); i++) {
    termui::terminal.CursorDown(line_spacing + 1).CursorLeft(_width - 4);

    kan::tui::JobCell cell = JobCell(_column.Jobs()[i]).width(_width - 2).blur();

    if (_focussed && i == _active_row) {
      cell.focus();
    }

    cell.render();

    down_set -= 4;
  }

  if (down_set > 0) {
    termui::terminal.CursorDown(down_set);
  }
}

} // namespace tui
} // namespace kan
