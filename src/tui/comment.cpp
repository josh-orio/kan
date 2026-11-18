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

Comment &Comment::maxLines(std::optional<int> n) {
  _max_lines = n;
  return *this;
}

unsigned int Comment::textLines() { return termui::Text(_comment.text).width(std::max(1, *_width - 2)).size().second; }

bool Comment::truncated() { return _max_lines.has_value() && textLines() > *_max_lines; }

std::pair<unsigned int, unsigned int> Comment::size() {
  bool has_w = _width.has_value(), has_h = _height.has_value();
  int  w = *_width, h = *_height;

  if (truncated()) {
    return {w, 1 + 1 + *_max_lines + 1}; // header, gap, the lines shown, the more note
  }

  auto [width, height] =
      termui::PaddedText(_comment.text).setPadding({2, 0, 1, 0}).style(_style).width(w).height(has_h ? std::optional<int>(h - 1) : std::nullopt).size();

  return {width, height + 1};
}

void Comment::render() {
  bool has_w = _width.has_value(), has_h = _height.has_value();
  int  w = *_width, h = *_height;

  auto bold_style = termui::Style(_style.foregroundColor, _style.backgroundColor, {termui::SGR::Bold});

  termui::TextRow(w)
      .paddingStyle(_style)
      .add(termui::Text(_comment.creator.Name(), bold_style))
      .add(termui::Text("commented", _style))
      .add(termui::Text(_comment.created.toString(), _style).align(termui::Text::Alignment::Right), 20)
      .render();

  termui::terminal.CursorDown(1).CursorLeft(w);

  if (truncated()) {
    auto hidden = textLines() - *_max_lines;

    auto [row, col] = termui::terminal.GetFutureCursorPosition();
    termui::PaddedText(_comment.text).setPadding({2, 0, 1, 0}).style(_style).width(w).height(*_max_lines + 1).render();

    termui::terminal.MoveCursor(row + *_max_lines + 1, col);
    auto faint = termui::Style(_style.foregroundColor, _style.backgroundColor, {termui::SGR::Faint});
    termui::PaddedText(std::format("… {} more line{} · ↵ to read", hidden, hidden == 1 ? "" : "s"), faint).setPadding({2, 0, 0, 0}).width(w).height(1).render();
    return;
  }

  termui::PaddedText(_comment.text).setPadding({2, 0, 1, 0}).style(_style).width(w).height(has_h ? std::optional<int>(h - 1) : std::nullopt).render();
}

} // namespace tui
} // namespace kan
