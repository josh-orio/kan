#ifndef TUI_JOB_WINDOW_HPP
#define TUI_JOB_WINDOW_HPP

#include <kan/core/job.hpp>
#include <kan/tui/comment-list.hpp>
#include <kan/tui/new-comment-window.hpp>
#include <kan/tui/tab-bar.hpp>
#include <termui/termui.hpp>

namespace kan {
namespace tui {

class JobDetails {
public:
  enum class Focus { Status, Description, Archive, Delete };

  JobDetails(kan::Job job);

  JobDetails &size(uint w, uint h);

  JobDetails &CursorUp();
  JobDetails &CursorDown();
  JobDetails &CursorLeft();
  JobDetails &CursorRight();

  Focus Focussed();

  void render();

private:
  uint _width, _height;

  kan::Job _job;

  Focus _selected_field = Focus::Status;
};

class Window : termui::Interface {
  uint _width = 80, _height = 40;

  tui::TabBar      _tab_bar = TabBar({"Job", "Comments", "Issues", "History"});
  tui::JobDetails  _job_detail;
  tui::CommentList _comment_list;

  kan::Job _job;
  bool     _modified = false;

  void display();
  void process_input();

public:
  Window(kan::Job job);

  std::pair<bool, kan::Job> show();
};

} // namespace tui
} // namespace kan

#endif
