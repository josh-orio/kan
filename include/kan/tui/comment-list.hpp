#ifndef TUI_COMMENTLIST_HPP
#define TUI_COMMENTLIST_HPP

#include <kan/core.hpp>

namespace kan {
namespace tui {

class CommentList {
public:
  CommentList(Comments comments);

  CommentList &size(unsigned int w, unsigned int h);
  CommentList &width(unsigned int w);
  CommentList &height(unsigned int h);

  void ReloadComments(Comments comments);

  void CursorUp(unsigned int count = 1);
  void CursorDown(unsigned int count = 1);

  unsigned int GetCursor();

  void render();

private:
  kan::Comments _comments;
  unsigned int  _w, _h;
  unsigned int  _visible_comments;
  unsigned int  _start_line;
  unsigned int  _cursor;
  unsigned int  _cell_height = 3;
  unsigned int  _line_spacing = 1;

  void internal_update();
};

} // namespace tui
} // namespace kan

#endif
