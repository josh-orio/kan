#ifndef TUI_EDITOR_WINDOW_HPP
#define TUI_EDITOR_WINDOW_HPP

#include <kan/core/job.hpp>
#include <kan/tui/comment-list.hpp>
#include <kan/tui/new-comment-window.hpp>
#include <kan/tui/tab-bar.hpp>
#include <termui/termui.hpp>

namespace kan {
namespace tui {

class EditWindow : termui::Interface {
private:
  termui::string _content;
  uint           _width, _height;

  void display();
  void process_input();

public:
  EditWindow(std::string content);

  EditWindow &width(unsigned int n);
  EditWindow &height(unsigned int n);

  std::string show();
};

} // namespace tui
} // namespace kan

#endif