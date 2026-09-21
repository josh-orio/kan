#ifndef TUI_HISTORY_VIEWER_HPP
#define TUI_HISTORY_VIEWER_HPP

#include <kan/core.hpp>

namespace kan {
namespace tui {

class HistoryViewer {
public:
  HistoryViewer(History history);

  HistoryViewer &size(unsigned int w, unsigned int h);
  HistoryViewer &width(unsigned int w);
  HistoryViewer &height(unsigned int h);

  // void ReloadComments(Comments comments);
  void Reload(History h);

  void CursorUp(unsigned int count = 1);
  void CursorDown(unsigned int count = 1);

  unsigned int GetCursor();

  void render();

private:
  kan::History _history;

  unsigned int _w, _h;
  unsigned int _visible_rows;
  unsigned int _start_line;
  unsigned int _cursor;
  unsigned int _cell_height  = 2;
  unsigned int _line_spacing = 1;

  void internal_update();
};

} // namespace tui
} // namespace kan

#endif
