#ifndef TUI_COMMIT_LIST_HPP
#define TUI_COMMIT_LIST_HPP

#include <kan/core.hpp>

namespace kan {
namespace tui {

// one line per commit: title, rest of the message, short hash, date
class CommitList {
public:
  CommitList(Commits commits);

  CommitList &size(unsigned int w, unsigned int h);

  void Reload(Commits commits);

  void CursorUp(unsigned int count = 1);
  void CursorDown(unsigned int count = 1);

  void GotoTop();
  void GotoBottom();

  bool         Empty();
  kan::Commit  Selected();
  unsigned int GetCursor();

  void render();

private:
  std::vector<kan::Commit> _commits;

  unsigned int _w = 0, _h = 0;
  unsigned int _visible_rows = 0;
  unsigned int _start_line   = 0;
  unsigned int _cursor       = 0;

  void internal_update();
};

} // namespace tui
} // namespace kan

#endif
