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
  auto base_style = board.styleMap.at("default");

  int tamper = 15 * (base_style.backgroundColor.isDark() ? 1 : -1);

  auto row_style = base_style;
  row_style.backgroundColor.rgb.r += tamper;
  row_style.backgroundColor.rgb.g += tamper;
  row_style.backgroundColor.rgb.b += tamper;

  auto row_style_bold       = row_style;
  row_style_bold.attributes = {termui::SGR::Bold};

  termui::Text(" " + _job.title).style(_focussed ? row_style_bold : termui::Style()).size(_width - 11, 1).render();

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
