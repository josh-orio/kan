#ifndef TUI_NEWCOMMENTWINDOW_HPP
#define TUI_NEWCOMMENTWINDOW_HPP

#include <kan/core.hpp>
#include <string>
#include <termui/termui.hpp>

namespace kan {
namespace tui {

class NewCommentWindow : termui::Interface {
private:
  // termui::string _comment;
  // kan::User      _user;
  // kan::Timestamp _time;
  kan::Comment _comment;

  uint _width = 60, _height = 9;

  enum class Focus { Text, Save };
  Focus _selected_field = NewCommentWindow::Focus::Text;

  void display();
  void process_input();

public:
  NewCommentWindow();

  std::pair<bool, kan::Comment> show();
};

} // namespace tui
} // namespace kan

#endif