#ifndef TUI_JOB_DETAILS_HPP
#define TUI_JOB_DETAILS_HPP

#include <kan/core/commits.hpp>
#include <kan/core/job.hpp>
#include <kan/tui/comment-list.hpp>
#include <kan/tui/new-comment-window.hpp>
#include <kan/tui/tab-bar.hpp>
#include <termui/termui.hpp>

namespace kan {
namespace tui {

class JobDetails : termui::Interface {
public:
  enum class Focus { Status, Priority, Assignee, EventList };

  JobDetails(kan::Job job);

  JobDetails &size(uint w, uint h);

  JobDetails &CursorUp();
  JobDetails &CursorDown();
  JobDetails &CursorLeft();
  JobDetails &CursorRight();

  JobDetails &GotoTop();
  JobDetails &GotoBottom();

  void Reload(Job job);

  JobDetails &editMode(bool x);

  Focus Focussed();

  void render();

private:
  uint _width, _height;

  kan::Job _job;

  std::vector<std::variant<kan::Comment, kan::Commit>> _events;

  Focus _focus = Focus::Status;

  unsigned int _visible_events;
  unsigned int _start_line = 0;
  unsigned int _cursor     = 0;

  bool _edit_mode = false;

  void internal_update();
};

} // namespace tui
} // namespace kan

#endif
