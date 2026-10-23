#include <kan/tui.hpp>

namespace kan {
namespace tui {

JobCell::JobCell(Job job) : _job(job) {}

JobCell &JobCell::width(int n) {
  _width = n;
  return *this;
}

JobCell &JobCell::focus() {
  _focussed = true;
  return *this;
}

JobCell &JobCell::blur() {
  _focussed = false;
  return *this;
}

JobCell &JobCell::setFocus(bool x) {
  _focussed = x;
  return *this;
}

void JobCell::render() {

    auto color = termui::terminal.BackgroundColor();

    int tamper = 15 * (termui::terminal.HasDarkBackground() ? 1 : -1);

    color.rgb.r += tamper;
    color.rgb.g += tamper;
    color.rgb.b += tamper;

  auto row_style= termui::Style(termui::Color(), color);
  auto row_style_bold= termui::Style(termui::Color(), color, {termui::SGR::Bold});

  termui::Text(" " + _job.title).style(_focussed ? row_style_bold: termui::Style()).size(_width - 11, 1).render();
  // , _style_map.at(_focussed ? "title_focus" : "title_blur"), _width - 11, 1).render();

  termui::PaddedText("#" + _job.uuid.substr(_job.uuid.size() - 8))
      .setPadding(termui::PaddingStyle::label)
      .style(_focussed ? row_style: termui::Style())
      .size(11, 1)
      .render();

  termui::terminal.CursorLeft(_width).CursorDown(1);

  termui::PaddedText(_job.description)
      .setPadding(termui::PaddingStyle::label)
      .style(_focussed ? row_style: termui::Style())
      .size(_width, 1)
      .render();
}

} // namespace tui
} // namespace kan
