#ifndef TUI_COMMIT_HPP
#define TUI_COMMIT_HPP

#include <kan/core.hpp>
#include <termui/termui.hpp>

namespace kan {
namespace tui {

class Commit {
private:
  kan::Commit        _commit;
  std::optional<int> _width, _height;
  termui::Style      _style;

public:
  Commit(kan::Commit commit);

  Commit &width(std::optional<int> n);
  Commit &height(std::optional<int> n);

  Commit &style(termui::Style s);

  std::pair<unsigned int, unsigned int> size();

  void render();
};

} // namespace tui
} // namespace kan

#endif
