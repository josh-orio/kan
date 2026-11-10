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

  // auto [width, height] = termui::Text(_commit.message).width(w).height(has_h ? std::optional<int>(h - 1) : std::nullopt).size();
 auto [width, height] = termui::PaddedText(_commit.message).setPadding({2,0,1,0}).style(_style).width(w).height(has_h ? std::optional<int>(h - 1) : std::nullopt).size();

  return {width, height + 1};
}

void Commit::render() {
  bool has_w = _width.has_value(), has_h = _height.has_value();
  int  w = *_width, h = *_height;

  auto bold_style = termui::Style(_style.foregroundColor, _style.backgroundColor, {termui::SGR::Bold});

  termui::TextRow(w)
      .paddingStyle(_style)
      .add(termui::Text(_commit.creator.Name(), bold_style))
      .add(termui::Text("referenced this job in commit", _style))
      .add(termui::Text(_commit.hash.substr(0, 7), bold_style))
      .add(termui::Text(_commit.created.toString(), _style).align(termui::Text::Alignment::Right), 20)
      .render();

  termui::terminal.CursorDown(1).CursorLeft(w);

  // termui::Text(_commit.message).style(_style).width(w).height(has_h ? std::optional<int>(h - 1) : std::nullopt).render();
  termui::PaddedText(_commit.message).setPadding({2,0,1,0}).style(_style).width(w).height(has_h ? std::optional<int>(h - 1) : std::nullopt).render();
}

} // namespace tui
} // namespace kan
