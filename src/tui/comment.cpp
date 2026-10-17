#include <kan/tui.hpp>

namespace kan {
namespace tui {

Comment::Comment(kan::Comment comment) : _comment(comment), _width(std::nullopt), _height(std::nullopt), _style() {}

Comment &Comment::width(std::optional<int> n) {
  _width = n;
  return *this;
}

Comment &Comment::height(std::optional<int> n) {
  _height = n;
  return *this;
}

Comment &Comment::style(termui::Style s) {
  _style = s;
  return *this;
}

std::pair<unsigned int, unsigned int> Comment::size() {
  bool has_w = _width.has_value(), has_h = _height.has_value();
  int  w = *_width, h = *_height;

  auto [width, height] = termui::Text(_comment.text).width(w).height(has_h ? std::optional<int>(h - 1) : std::nullopt).size();

  return {width, height + 1};
}

void Comment::render() {
  bool has_w = _width.has_value(), has_h = _height.has_value();
  int  w = *_width, h = *_height;

  termui::Text(_comment.text).style(_style).width(w).height(has_h ? std::optional<int>(h - 1) : std::nullopt).render();

  termui::terminal.CursorDown(1).CursorLeft(w);

  termui::Text(_comment.creator.Name()).style(termui::Style(_style.Foreground(), _style.Background(), {termui::SGR::Bold})).size(w - 20, 1).render();
  termui::Text(_comment.created.toString()).style(_style).size(20, 1).align(termui::Text::Alignment::Right).render();
}

} // namespace tui
} // namespace kan
