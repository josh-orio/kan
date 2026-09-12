#include <kan/tui.hpp>

namespace kan {
namespace tui {

Window::Window(kan::Job job) : _job(job), _job_detail(job), _comment_list(job.comments) {}

void Window::display()
{
  auto terminal_half_w = termui::terminal.halfWidth();
  auto terminal_half_h = termui::terminal.halfHeight();

  auto window_start_row = terminal_half_h - (_height / 2);
  auto window_start_col = terminal_half_w - (_width / 2);

  auto tab = _tab_bar.GetTab();

  // box
  termui::terminal.MoveCursor(window_start_row, window_start_col);
  termui::Box(_width, _height, termui::Borders::rounded, termui::Styles::faint).render();

  // title
  termui::terminal.MoveCursor(window_start_row + 1, window_start_col + 2);
  termui::Text(_job.title, termui::Styles::bold).width(_width - 4).height(1).render();

  // tab bar
  termui::terminal.MoveCursor(window_start_row + 3, window_start_col + 2);
  // termui::Text(_job._title, termui::Styles::bold).width(_width - 4).height(1).render();
  _tab_bar.width(_width - 4).render();

  // clear content area
  termui::terminal.MoveCursor(window_start_row + 5, window_start_col + 2);
  termui::Text("").style(termui::Styles::none).width(_width - 4).height(_height - 6).render();
  // termui::Text("").style(termui::Style(termui::Color::Inherit(), termui::Colors::Red)).width(_width - 4).height(_height - 6).render();

  termui::terminal.MoveCursor(window_start_row + 5, window_start_col + 2);

  termui::terminal.flush(); // have to do this so child components can get an accurate cursor position

  if (tab == 0) {
    // auto [row,col]=termui::terminal.GetCursorPosition();
    _job_detail.size(_width - 4, _height - 6).render();
  }
  if (tab == 1) {
    _comment_list.size(_width - 4, _height - 6).render();
  }
  if (tab == 2) {
    // _issue_list.size(_width - 4, _height - 6).render();
  }

  termui::terminal.flush();
}

void Window::process_input()
{
  auto inp = termui::terminal.read();

  auto tab = _tab_bar.GetTab();

  if (inp == termui::key::ESC || inp == "q") {
    state = termui::Interface::State::Exit;
  }
  else if (inp == termui::key::Tab) {
    _tab_bar.TabIncrement();
  }
  else if (inp == termui::key::ShiftTab) {
    _tab_bar.TabDecrement();
  }
  else if (inp == termui::key::U_ARROW) {
    if (tab == 0) {
      _job_detail.CursorUp();
    }
    else if (tab == 1) {
      _comment_list.CursorUp();
    }
  }
  else if (inp == termui::key::D_ARROW) {
    if (tab == 0) {
      _job_detail.CursorDown();
    }
    else if (tab == 1) {
      _comment_list.CursorDown();
    }
  }
  else if (inp == termui::key::L_ARROW) {
    if (tab == 0) {
      _job_detail.CursorLeft();
    }
  }
  else if (inp == termui::key::R_ARROW) {
    if (tab == 0) {
      _job_detail.CursorRight();
    }
  }
  else if (inp == "c") {
    if (tab == 1) {
      auto [success, comment] = NewCommentWindow().show();

      if (success) {
        // _job.comments._comments.push_back(comment);
        _job.comments.addComment(comment);
        // _comment_list = CommentList(_job._comments);// reload comments - very inefficient
        _comment_list.ReloadComments(_job.comments);
        _modified = true;
      }
    }
  }
  else if (inp == "o" || inp == termui::key::ENTER) {
    if (tab == 0) {
      if (_job_detail.Focussed() == JobDetails::Focus::Status) {
        // mini menu
        std::vector<std::string> statuses = board.Statuses();

        auto [state, cursor] = tui::MiniMenu("Status", statuses).show(); // how to dynamically get this - create board singleton?

        if (state == termui::Interface::State::Submit) {
          _job.status = statuses.at(cursor);
          _job_detail = JobDetails(_job);
          _modified = true;
        }
      }
    }
    if (tab == 1) {
      ViewCommentWindow(_job.comments.at(_comment_list.GetCursor())).show();
    }
  }
}

std::pair<bool, kan::Job> Window::show()
{
  state = State::Continue;

  do {
    display();
    process_input();

  } while (state == State::Continue);

  return {_modified, _job};
}

JobDetails::JobDetails(kan::Job job) : _job(job) {}

JobDetails &JobDetails::size(uint w, uint h)
{
  _width = w;
  _height = h;
  return *this;
}

JobDetails &JobDetails::CursorUp()
{
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

JobDetails &JobDetails::CursorDown()
{
  if (_selected_field == Focus::Status) {
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

JobDetails &JobDetails::CursorLeft()
{
  if (_selected_field == Focus::Delete) {
    _selected_field = Focus::Archive;
  }

  return *this;
}

JobDetails &JobDetails::CursorRight()
{
  if (_selected_field == Focus::Archive) {
    _selected_field = Focus::Delete;
  }

  return *this;
}

JobDetails::Focus JobDetails::Focussed() { return _selected_field; }

void JobDetails::render()
{
  auto [row, col] = termui::terminal.GetCursorPosition();
  // status indicator
  if (_selected_field == Focus::Status) {
    termui::terminal.write("·");
  }

  // status
  termui::terminal.MoveCursor(row, col + 2);
  termui::Text(_job.status, ::tui::stl(::tui::clrs::Red, ::tui::clr::Inherit())).width(_width - 2).height(1).render();

  // desc indicator
  if (_selected_field == Focus::Description) {
    termui::terminal.MoveCursor(row + 2, col);
    termui::Line(termui::Line::Direction::Vertical, _height - 10).render();
  }

  // job description
  termui::terminal.MoveCursor(row + 2, col + 2);
  termui::Text(_job.description).width(_width - 6).height(_height - 10).render();

  // created
  termui::terminal.MoveCursor(row + _height - 7, col);
  termui::Text("Created: ").style(::tui::stls::bold).width(9).render();
  Timestamp t;
  t.time = _job.created;
  termui::Text(_job.creator.toString() + "\n" + t.toString()).width(_width - 6 - 9).height(2).render();

  // updated
  termui::terminal.MoveCursor(row + _height - 4, col);
  termui::Text("Updated: ").style(::tui::stls::bold).width(9).render();

  t.time = _job.updated;

  termui::Text(_job.creator.toString() + "\n" + t.toString()).width(_width - 6 - 9).height(2).render();

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
