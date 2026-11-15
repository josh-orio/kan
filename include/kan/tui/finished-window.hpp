#ifndef TUI_FINISHED_WINDOW_HPP
#define TUI_FINISHED_WINDOW_HPP

#include <kan/core.hpp>
#include <termui/termui.hpp>

namespace kan {
namespace tui {

// full screen log of finished jobs grouped by when they were finished (today, yesterday, this week, earlier)
class FinishedWindow : termui::Interface {
public:
  FinishedWindow() = default;

  void show();

private:
  unsigned int _width = 80, _height = 40;
  unsigned int _start_line = 0; // first visible line, lines include the group headers
  unsigned int _cursor     = 0; // index into the matching jobs

  std::string _query;
  bool        _searching = false;

  std::vector<kan::Job> matching(); // finished jobs that match the search, newest first

  void display();
  void process_input();
};

} // namespace tui
} // namespace kan

#endif
