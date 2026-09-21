#ifndef TUI_NEWISSUEWINDOW_HPP
#define TUI_NEWISSUEWINDOW_HPP

#include <kan/core.hpp>
#include <termui/termui.hpp>

namespace kan {
namespace tui {

class NewIssueWindow : termui::Interface {
private:
  kan::Issue _issue;

  uint _width = 60, _height = 15;

  enum class Focus { Text, Save };
  Focus _selected_field = NewIssueWindow::Focus::Text;

  void display();
  void process_input();

public:
  NewIssueWindow();

  std::pair<bool, kan::Issue> show();
};

} // namespace tui
} // namespace kan

#endif