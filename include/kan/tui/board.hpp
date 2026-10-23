#ifndef TUI_BOARD_HPP
#define TUI_BOARD_HPP

#include <format>
#include <kan/core.hpp>
#include <kan/tui/issue-list.hpp>
#include <kan/tui/tab-bar.hpp>
#include <kan/tui/column.hpp>
#include <vector>

namespace kan {
namespace tui {

class Board : termui::Interface {
private:
  uint _width, _height;
  uint _active_col, _active_row;

  tui::TabBar _tab_bar = TabBar({"Jobs", "Issues"});

  std::vector<tui::Column> _columns;
  // tui::IssueList issue_list;

  void display();
  void process_input();
  void internal_update();

public:
  Board();
  // Board(kan::Board &board);

  void load();

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

  // void render();
  void show();
};

} // namespace tui
} // namespace kan

#endif

// class JobWindow : termui::Interface {
//   uint _width = 80, _height = 40;

//   tui::TabBar        _tab_bar = TabBar({"Details", "Comments", "Issues", "History"});
//   tui::JobDetails    _job_detail;
//   tui::CommentList   _comment_list;
//   tui::IssueList     _issue_list;
//   tui::HistoryViewer _history_viewer;

//   kan::Job _job;
//   bool     _modified = false;

//   void display();
//   void process_input();

// public:
//   JobWindow(kan::Job job);

//   std::pair<bool, kan::Job> show();
// };
