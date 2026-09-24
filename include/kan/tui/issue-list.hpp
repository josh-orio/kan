#ifndef TUI_ISSUELIST_HPP
#define TUI_ISSUELIST_HPP

#include <kan/core.hpp>

namespace kan {
namespace tui {

class IssueList {
public:
  IssueList(Issues issues);

  IssueList &size(unsigned int w, unsigned int h);
  IssueList &width(unsigned int w);
  IssueList &height(unsigned int h);

  // void ReloadIssues(Issues issues);
  void ReloadIssues();

  void CursorUp(unsigned int count = 1);
  void CursorDown(unsigned int count = 1);

  unsigned int GetCursor();

  void render();

private:
  kan::Issues  _issues;
  unsigned int _w, _h;
  unsigned int _visible_issues;
  unsigned int _start_line;
  unsigned int _cursor;
  unsigned int _cell_height  = 3;
  unsigned int _line_spacing = 1;

  void internal_update();
};

} // namespace tui
} // namespace kan

#endif
