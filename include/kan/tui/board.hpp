#ifndef TUI_BOARD_HPP
#define TUI_BOARD_HPP

#include <format>
#include <kan/core.hpp>
#include <vector>

namespace kan {
namespace tui {

class Board {
private:
  uint _width, _height;
  uint _active_col, _active_row;

public:
  Board() = default;
  Board(kan::Board &board);

  Board &width(uint w);
  Board &height(uint h);

  uint ActiveRow();
  uint ActiveColumn();

  void Up(uint n = 1);
  void Down(uint n = 1);
  void Left(uint n = 1);
  void Right(uint n = 1);

  Board &ActiveRow(uint row);
  Board &ActiveColumn(uint col);

  void render();
};

} // namespace tui
} // namespace kan

#endif
