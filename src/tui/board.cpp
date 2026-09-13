#include <kan/tui.hpp>

#include <format>

namespace kan {
namespace tui {

Board::Board(kan::Board &board) : /* _board(board), */ _active_col(0), _active_row(0) {}

Board &Board::width(uint n) {
  _width = n;
  return *this;
}

Board &Board::height(uint n) {
  _height = n;
  return *this;
}

uint Board::ActiveColumn() { return _active_col; }

uint Board::ActiveRow() { return _active_row; }

void Board::Up(uint n) { _active_row = std::max<int>(0, _active_row - n); }

void Board::Down(uint n) { _active_row = std::min<int>(board.Jobs(_active_col).size() == 0 ? 0 : board.Jobs(_active_col).size() - 1, _active_row + n); }

void Board::Left(uint n) {
  _active_col = std::max<int>(0, _active_col - n);

  if (_active_row > board.Jobs(_active_col).size()) {
    _active_row = board.Jobs(_active_col).size();
  }
}

void Board::Right(uint n) {
  _active_col = std::min<int>(board.Columns().size() - 1, _active_col + n);

  if (_active_row > board.Jobs(_active_col).size()) {
    _active_row = board.Jobs(_active_col).size();
  }
}

Board &Board::ActiveColumn(uint col) {
  _active_col = col;
  return *this;
}

Board &Board::ActiveRow(uint row) {
  _active_row = row;
  return *this;
}

void Board::render() {
  termui::Text(board.Title(), termui::Styles::bold).height(1).width(20).render();
  // termui::terminal.write(std::format("{} {}", _active_col, _active_row));
  termui::terminal.CursorDown(2).CursorLeft(20);

  int w         = _width;
  int cols      = board.Columns().size();
  int col_width = _width <= 0 ? 0 : [w, cols]() {
    int overhead    = 4;
    int col_spacing = 1;

    // cols = (h - overhead) / (line_height + line_spacing);
    // (line_height + line_spacing) = (h - overhead) / cols;
    // line_height = ((h - overhead) / cols) - line_spacing ;
    return ((w - overhead) / cols) - col_spacing;
  }();

  int count = 0;
  // for (auto col : _board._columns) {
  for (int i = 0; i < board.Columns().size(); i++) {
    kan::tui::Column column(board.Column(i));

    if (i == _active_col) {
      if (_active_row > board.Jobs(i).size()) {
        _active_row = board.Jobs(i).size();
      }

      column.focus(_active_row);
    }
    else {
      column.blur();
    }

    column.width(col_width).height(_height - 4);
    column.render();

    count++;

    if (count < board.Columns().size()) {
      termui::terminal.CursorUp(_height - 4 - 2).CursorRight(2);
    }
  }

  // termui::terminal.ShowCursor();
  // termui::terminal.CursorDown(3).CursorLeft((col_width * _board._columns.size()) + 1);
  termui::terminal.MoveCursor(termui::terminal.height(), 2);

  ControlStrip({
                   {"←↑↓→", "move"},
                   {"o/↵", "open"},
                   {"n", "new job"},
                   {"?", "help"},
                   {"s", "settings"},
                   {"q", "quit"},
               })
      .width(_width - 2)
      .render();
}

} // namespace tui
} // namespace kan
