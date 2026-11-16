#ifndef TUI_BOARD_HPP
#define TUI_BOARD_HPP

#include <format>
#include <kan/core.hpp>
#include <kan/tui/tab-bar.hpp>
#include <kan/tui/column.hpp>
#include <vector>

namespace kan {
namespace tui {

bool jobMatches(const kan::Job &job, const std::string &query); // the board's search: title, description, #number

class Board : termui::Interface {
private:
  uint _width, _height;
  uint _active_col, _active_row;

  std::vector<tui::Column> _columns;

  std::string _query;             // search text, matched against job title, description and number
  bool        _searching = false; // typing into the search bar

  std::vector<unsigned int> visibleRows(unsigned int col); // indices of the jobs in a column that match the search
  void                      setQuery(std::string query);

  // live refresh - the board is checked about once a second while it waits for a key
  kan::Watcher _kan_watch; // kan.yaml, jobs/, contributors.yaml
  kan::Watcher _git_watch; // HEAD and branches, for the ⎇ markers
  bool         _redraw = true;

  bool refresh();        // true if anything on screen needs redrawing
  void rebuildColumns(); // after board.columns was replaced, added to or removed from

  void display();
  void process_input();
  void internal_update();

public:
  Board();


  static Board &instance();

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

  void show();
};

} // namespace tui
} // namespace kan

#endif
