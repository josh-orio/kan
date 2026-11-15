#ifndef TUI_DIFF_VIEWER_HPP
#define TUI_DIFF_VIEWER_HPP

#include <optional>
#include <string>
#include <vector>

namespace kan {
namespace tui {

// scrollable diff, added lines in green and removed lines in red.
// shows either the unified diff or old/new side by side
class DiffViewer {
public:
  DiffViewer(std::vector<std::string> lines = {});

  DiffViewer &size(unsigned int w, unsigned int h);

  DiffViewer &sideBySide(bool x);
  bool        SideBySide();

  void ScrollUp(unsigned int count = 1);
  void ScrollDown(unsigned int count = 1);

  void GotoTop();
  void GotoBottom();

  void render();

private:
  // a row of the side by side view - a hunk header spans both sides, otherwise either side may be empty
  struct Row {
    std::optional<std::string> left, right;
    std::string                header;
  };

  std::vector<std::string> _lines;
  std::vector<Row>         _rows;

  bool _side_by_side = false;

  unsigned int _w = 0, _h = 0;
  unsigned int _start_line = 0;

  unsigned int rowCount();
  unsigned int maxStart();

  void renderUnified(unsigned int row, unsigned int col);
  void renderSideBySide(unsigned int row, unsigned int col);
};

} // namespace tui
} // namespace kan

#endif
