#include <kan/tui.hpp>

namespace kan {
namespace tui {

std::string priorityStr(Job::Priority p) {
  switch (p) {
  case Job::Priority::Low:
    return "Low";
    break;
  case Job::Priority::Normal:
    return "Normal";
    break;
  case Job::Priority::High:
    return "High";
    break;
  }
}

std::string priorityIcon(Job::Priority p) {
  switch (p) {
  case Job::Priority::Low:
    return "∨";
    break;
  case Job::Priority::Normal:
    return "-";
    break;
  case Job::Priority::High:
    return "∧";
    break;
  }
}

JobDetails::JobDetails(kan::Job job) : _job(job) {}

JobDetails &JobDetails::size(uint w, uint h) {
  _width  = w;
  _height = h;
  return *this;
}

JobDetails &JobDetails::CursorUp() {
  if (_selected_field == Focus::Description) {
    _selected_field = Focus::Status;
  }
  else if (_selected_field == Focus::Archive) {
    _selected_field = Focus::Description;
  }
  else if (_selected_field == Focus::Delete) {
    _selected_field = Focus::Description;
  }

  return *this;
}

JobDetails &JobDetails::CursorDown() {
  if (_selected_field == Focus::Status) {
    _selected_field = Focus::Description;
  }
  else if (_selected_field == Focus::Priority) {
    _selected_field = Focus::Description;
  }
  else if (_selected_field == Focus::Description) {
    _selected_field = Focus::Archive;
  }
  else if (_selected_field == Focus::Archive) {
    _selected_field = Focus::Delete;
  }

  return *this;
}

JobDetails &JobDetails::CursorLeft() {
  if (_selected_field == Focus::Priority) {
    _selected_field = Focus::Status;
  }
  else if (_selected_field == Focus::Delete) {
    _selected_field = Focus::Archive;
  }

  return *this;
}

JobDetails &JobDetails::CursorRight() {
  if (_selected_field == Focus::Status) {
    _selected_field = Focus::Priority;
  }
  else if (_selected_field == Focus::Archive) {
    _selected_field = Focus::Delete;
  }

  return *this;
}

JobDetails::Focus JobDetails::Focussed() { return _selected_field; }

void JobDetails::render() {
  auto [row, col] = termui::terminal.GetCursorPosition();

  // status indicator
  if (_selected_field == Focus::Status) {
    termui::terminal.write("·");
  }

  // status
  termui::terminal.MoveCursor(row, col + 2);
  termui::Text(_job.status, ::tui::stl(::tui::clrs::Red, ::tui::clr::Inherit())).width(_width - 2).height(1).render();

  // priority (symbol + text)
  termui::terminal.MoveCursor(row, col + _width - 10);
  termui::terminal.write(priorityIcon(_job.priority));

  termui::terminal.MoveCursor(row, col + _width - 8);
  termui::Text(priorityStr(_job.priority)).style(termui::Style(termui::Color::Inherit(), termui::Colors::Red, {termui::SGR::Bold})).width(6).render();

  // priority indicator
  if (_selected_field == Focus::Priority) {
    termui::terminal.MoveCursor(row, col + _width - 1);
    termui::terminal.write("·");
  }

  // desc indicator
  if (_selected_field == Focus::Description) {
    termui::terminal.MoveCursor(row + 2, col);
    termui::Line(termui::Line::Direction::Vertical, _height - 10).render();
  }

  // job description
  termui::terminal.MoveCursor(row + 2, col + 2);
  termui::Text(_job.description).width(_width - 4).height(_height - 10).style(termui::Style(termui::Color::Inherit(), termui::Colors::Red)).render();

  // created
  termui::terminal.MoveCursor(row + _height - 7, col);
  termui::Text("Created: ").style(::tui::stls::bold).width(9).render();
  termui::Text(_job.creator.toString() + "\n" + _job.created.toString()).width(_width - 6 - 9).height(2).render();

  // updated
  termui::terminal.MoveCursor(row + _height - 4, col);
  termui::Text("Updated: ").style(::tui::stls::bold).width(9).render();
  termui::Text(_job.creator.toString() + "\n" + _job.updated.toString()).width(_width - 6 - 9).height(2).render();

  // help bar
  termui::terminal.MoveCursor(row + _height - 1, col);
  kan::tui::ControlStrip({
                             {"esc", "close"},
                             {"↑↓", "navigate"},
                             {"↵", "select"},
                         })
      .width(_width - 1 - 9 - 1 - 8)
      .render();

  // archive button
  termui::Button archive = termui::Button("Archive", termui::Style({termui::SGR::Bold, termui::SGR::ReverseVideo}), termui::Style()).width(9);

  if (_selected_field == Focus::Archive) {
    archive.focus();
  }
  else {
    archive.blur();
  }

  archive.render();

  termui::terminal.write(" ");

  // delete button
  termui::Button del = termui::Button("Delete", termui::Style(termui::Color::Inherit(), termui::Colors::Red, {termui::SGR::Bold}), termui::Style()).width(8);

  if (_selected_field == Focus::Delete) {
    del.focus();
  }
  else {
    del.blur();
  }

  del.render();
}

} // namespace tui
} // namespace kan
