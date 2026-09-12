#ifndef TUI_COLUMN_HPP
#define TUI_COLUMN_HPP

#include <kan/core.hpp>
#include <termui/termui.hpp>

namespace kan {
namespace tui {

class Column {
private:
  kan::Column _column;
  uint        _width, _height;
  bool        _focussed;
  uint        _active_row;

public:
  Column(kan::Column column);

  Column &width(uint n);
  Column &height(uint n);

  Column &focus(uint row);
  Column &blur();

  void render();
};

} // namespace tui
} // namespace kan

#endif
