#ifndef TUI_COLUMN_HPP
#define TUI_COLUMN_HPP

#include <kan/core.hpp>
#include <termui/termui.hpp>

namespace kan {
namespace tui {

class Column {
private:
  kan::Column *_column; // points into kan::board.columns so new/reloaded jobs show up
  unsigned int _width, _height;
  bool         _focussed;
  unsigned int _visible_rows;
  unsigned int _start_line;
  unsigned int _cursor;

  std::vector<unsigned int> _rows; // indices into the column's jobs that are shown (all of them unless searching)

  void internal_update();

public:
  Column(kan::Column &column);

  Column &filter(std::vector<unsigned int> rows);

  Column &width(uint n);
  Column &height(uint n);

  Column &focus();
  Column &blur();

  Column &cursorUp();
  Column &cursorDown();
  Column &setCursor(unsigned int row);

  Column &gotoTop();
  Column &gotoBottom();

  void render();
};

} // namespace tui
} // namespace kan

#endif
