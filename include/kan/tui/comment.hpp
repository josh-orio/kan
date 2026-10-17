#ifndef TUI_COMMENT_HPP
#define TUI_COMMENT_HPP

#include <kan/core.hpp>
#include <termui/termui.hpp>

namespace kan {
namespace tui {

class Comment {
private:
  kan::Comment       _comment;
  std::optional<int> _width, _height;
  termui::Style      _style;

public:
  Comment(kan::Comment comment);

  Comment &width(std::optional<int> n);
  Comment &height(std::optional<int> n);

  Comment &style(termui::Style s);

  std::pair<unsigned int, unsigned int> size();

  void render();
};

} // namespace tui
} // namespace kan

#endif
