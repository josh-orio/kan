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
  auto row_style = board.styleMap.at("highlight");

  auto row_style_bold       = row_style;
  row_style_bold.attributes = {termui::SGR::Bold};

  auto row_style_faint      = row_style;
  row_style_faint.attributes = {termui::SGR::Faint};

  // finished this session - crossed out and faint until kan closes
  auto strike = [&](termui::Style s) {
    if (_job.finished) {
      s.attributes = {termui::SGR::Faint, termui::SGR::CrossedOut};
    }
    return s;
  };

  termui::Text(" " + _job.title).style(strike(_focussed ? row_style_bold : termui::Style())).size(_width - 11, 1).render();

  termui::PaddedText("#" + std::to_string(_job.id))
      .setPadding(termui::PaddingStyle::label)
      .style(_focussed ? row_style_faint: termui::Styles::faint)
      .size(11, 1)
      .align(termui::Text::Alignment::Right)
      .render();

  termui::terminal.CursorLeft(_width).CursorDown(1);

  termui::PaddedText(_job.description)
      .setPadding(termui::PaddingStyle::label)
      .style(strike(_focussed ? row_style : termui::Style()))
      .size(_width, 1)
      .render();
}

} // namespace tui
} // namespace kan
