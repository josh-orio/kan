#ifndef TUI_JOB_CELL_HPP
#define TUI_JOB_CELL_HPP

#include <kan/core.hpp>
#include <termui/termui.hpp>

namespace kan {
namespace tui {

class Cell {
private:
  kan::Job _job;
  int      _width, _height = 2;
  bool     _focussed;

  termui::StyleMap _style_map = termui::StyleMap({
      {"title_focus", termui::Style({termui::SGR::Bold, termui::SGR::ReverseVideo})},
      {"title_blur", termui::Style({termui::SGR::Bold})},
      {"body_focus", termui::Style({termui::SGR::ReverseVideo})},
      {"body_blur", termui::Style()},
  });

public:
  Cell(kan::Job job);

  Cell &width(int n);

  Cell &focus();
  Cell &blur();

  void render();
};

} // namespace tui
} // namespace kan

#endif
