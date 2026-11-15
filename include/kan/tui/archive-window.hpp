#ifndef TUI_ARCHIVE_WINDOW_HPP
#define TUI_ARCHIVE_WINDOW_HPP

#include <kan/core.hpp>
#include <termui/termui.hpp>

namespace kan {
namespace tui {

// full screen list of archived jobs, where they can be restored to the board or deleted for good
class ArchiveWindow : termui::Interface {
public:
  ArchiveWindow() = default;

  void show();

private:
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
