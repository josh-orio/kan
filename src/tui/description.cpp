#include <kan/tui.hpp>

namespace kan {
namespace tui {

Description::Description(JobDescription description) : _description(description), _width(std::nullopt), _height(std::nullopt), _style() {}

Description &Description::width(std::optional<int> n) {
  _width = n;
  return *this;
}

Description &Description::height(std::optional<int> n) {
  _height = n;
  return *this;
}

Description &Description::style(termui::Style s) {
  _style = s;
  return *this;
}

std::pair<unsigned int, unsigned int> Description::size() {
  bool has_h = _height.has_value();
  int  w = *_width, h = *_height;

  auto text = _description.text.empty() ? std::string("No description provided.") : _description.text;

  auto [width, height] =
      termui::PaddedText(text).setPadding({2, 0, 1, 0}).style(_style).width(w).height(has_h ? std::optional<int>(h - 1) : std::nullopt).size();

  return {width, height + 1};
}

void Description::render() {
  bool has_h = _height.has_value();
  int  w = *_width, h = *_height;

  auto bold_style  = termui::Style(_style.foregroundColor, _style.backgroundColor, {termui::SGR::Bold});
  auto faint_style = termui::Style(_style.foregroundColor, _style.backgroundColor, {termui::SGR::Faint});

  termui::TextRow(w)
      .paddingStyle(_style)
      .add(termui::Text("Description", bold_style))
      .add(termui::Text(_description.created.toString(), _style).align(termui::Text::Alignment::Right), 20)
      .render();

  termui::terminal.CursorDown(1).CursorLeft(w);

  if (_description.text.empty()) {
    termui::PaddedText("No description provided.")
        .setPadding({2, 0, 1, 0})
        .style(faint_style)
        .width(w)
        .height(has_h ? std::optional<int>(h - 1) : std::nullopt)
        .render();
  }
  else {
    termui::PaddedText(_description.text).setPadding({2, 0, 1, 0}).style(_style).width(w).height(has_h ? std::optional<int>(h - 1) : std::nullopt).render();
  }
}

} // namespace tui
} // namespace kan
