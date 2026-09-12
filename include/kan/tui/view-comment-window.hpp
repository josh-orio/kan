#ifndef TUI_VIEWCOMMENTWINDOW_HPP
#define TUI_VIEWCOMMENTWINDOW_HPP

#include <kan/core.hpp>
#include <string>
#include <termui/termui.hpp>

namespace kan {
namespace tui {

class ViewCommentWindow : termui::Interface {
private:
kan::Comment _comment;
termui::Pager _pager;

  uint _width = 80, _height = 40;

  void display();
  void process_input();

public:
  ViewCommentWindow(kan::Comment comment);

  void show();
};

} // namespace tui
} // namespace kan

#endif