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

void JobCell::render() {
  // termui::Text(_job._title+ (_focussed ? " focussed": ""), _focussed ? _style_map.at("title_focus") : _style_map.at("title_blur"), _width - 12, 1).render();
  termui::Text(_job.title, _focussed ? _style_map.at("title_focus") : _style_map.at("title_blur"), _width - 12, 1).render();
  termui::Text(" #" + _job.uuid.substr(_job.uuid.size() - 8), _focussed ? _style_map.at("body_focus") : _style_map.at("body_blur"), 10, 1).render();

  termui::terminal.CursorLeft(_width - 2).CursorDown(1);

  termui::Text(_job.description).style(_focussed ? _style_map.at("body_focus") : _style_map.at("body_blur")).width(_width - 2).height(1).render();
}

} // namespace tui
} // namespace kan
