#ifndef TUI_COMMIT_WINDOW_HPP
#define TUI_COMMIT_WINDOW_HPP

#include <kan/core.hpp>
#include <termui/termui.hpp>

namespace kan {
namespace tui {

// full screen view of a commit and the files it changed, read from git when opened
class CommitWindow : termui::Interface {
public:
  CommitWindow(kan::Commit commit);

  void show();

private:
  kan::Commit                  _commit;
  std::vector<git::FileChange> _files;
  std::string                  _error; // set when git couldn't read the commit

  unsigned int _width = 80, _height = 40;
  unsigned int _visible_rows = 0;
  unsigned int _start_line   = 0;
  unsigned int _cursor       = 0;

  void display();
  void process_input();
};

} // namespace tui
} // namespace kan

#endif
