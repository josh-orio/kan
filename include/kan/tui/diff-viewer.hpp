#ifndef TUI_DIFF_VIEWER_HPP
#define TUI_DIFF_VIEWER_HPP

#include <string>
#include <vector>

namespace kan {
namespace tui {

// scrollable unified diff, added lines in green and removed lines in red
class DiffViewer {
public:
  DiffViewer(std::vector<std::string> lines = {});

  DiffViewer &size(unsigned int w, unsigned int h);

  void ScrollUp(unsigned int count = 1);
  void ScrollDown(unsigned int count = 1);

  void GotoTop();
  void GotoBottom();

  void render();

private:
  std::vector<std::string> _lines;

  unsigned int _w = 0, _h = 0;
  unsigned int _start_line = 0;

  unsigned int maxStart();
};

} // namespace tui
} // namespace kan

#endif
