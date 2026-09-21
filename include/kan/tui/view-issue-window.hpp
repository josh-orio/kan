#ifndef TUI_VIEWISSUEWINDOW_HPP
#define TUI_VIEWISSUEWINDOW_HPP

#include <kan/core.hpp>
#include <termui/termui.hpp>

namespace kan {
namespace tui {

class ViewIssueWindow : termui::Interface {
private:
  kan::Issue    _issue;
  termui::Pager _pager;

  uint _width = 80, _height = 40;

  void display();
  void process_input();

public:
  ViewIssueWindow(kan::Issue issue);

  termui::Interface::State show();
};

} // namespace tui
} // namespace kan

#endif