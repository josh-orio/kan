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
  termui::Text(" " + _job.title).style(_style_map.at(_focussed ? "title_focus" : "title_blur")).size(_width - 11, 1).render();
  // , _style_map.at(_focussed ? "title_focus" : "title_blur"), _width - 11, 1).render();

  termui::PaddedText("#" + _job.uuid.substr(_job.uuid.size() - 8))
      .setPadding(termui::PaddingStyle::label)
      .style(_style_map.at(_focussed ? "body_focus" : "body_blur"))
      .size(11, 1)
      .render();

  termui::terminal.CursorLeft(_width).CursorDown(1);

  // termui::Text(" "+_job.description).style(_style_map.at(_focussed ? "body_focus" : "body_blur")).width(_width ).height(1).render();
  termui::PaddedText(_job.description)
      .setPadding(termui::PaddingStyle::label)
      .style(_style_map.at(_focussed ? "body_focus" : "body_blur"))
      .size(_width, 1)
      .render();
}

} // namespace tui
} // namespace kan
