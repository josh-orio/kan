#include <kan/tui.hpp>

namespace kan {
namespace tui {

Commit::Commit(kan::Commit commit) : _commit(commit), _width(std::nullopt), _height(std::nullopt), _style() {}

Commit &Commit::width(std::optional<int> n) {
  _width = n;
  return *this;
}

Commit &Commit::height(std::optional<int> n) {
  _height = n;
  return *this;
}

Commit &Commit::style(termui::Style s) {
  _style = s;
  return *this;
}

std::pair<unsigned int, unsigned int> Commit::size() {
  bool has_w = _width.has_value(), has_h = _height.has_value();
  int  w = *_width, h = *_height;

  auto [width, height] = termui::Text(_commit.message).width(w).height(has_h ? std::optional<int>(h - 1) : std::nullopt).size();

  return {width, height + 1};
}

void Commit::render() {
  bool has_w = _width.has_value(), has_h = _height.has_value();
  int  w = *_width, h = *_height;

  termui::Text(_commit.message).style(_style).width(w).height(has_h ? std::optional<int>(h - 1) : std::nullopt).render();

  termui::terminal.CursorDown(1).CursorLeft(w);

  termui::Text(_commit.creator.Name()).style(termui::Style(_style.foregroundColor, _style.backgroundColor, {termui::SGR::Bold})).size(w - 20, 1).render();
  termui::Text(_commit.created.toString()).style(_style).size(20, 1).align(termui::Text::Alignment::Right).render();
}

} // namespace tui
} // namespace kan
