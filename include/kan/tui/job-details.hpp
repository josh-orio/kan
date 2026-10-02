#ifndef TUI_JOB_DETAILS_HPP
#define TUI_JOB_DETAILS_HPP

#include <kan/core/job.hpp>
#include <kan/tui/comment-list.hpp>
#include <kan/tui/new-comment-window.hpp>
#include <kan/tui/tab-bar.hpp>
#include <termui/termui.hpp>

namespace kan {
namespace tui {

class JobDetails : termui::Interface {
public:
  enum class Focus { Status, Priority, Description};

  JobDetails(kan::Job job);

  JobDetails &size(uint w, uint h);

  JobDetails &CursorUp();
  JobDetails &CursorDown();
  JobDetails &CursorLeft();
  JobDetails &CursorRight();

  void Reload(Job job);

  JobDetails &editMode(bool x);

  Focus Focussed();

  void render();

private:
  uint _width, _height;

  kan::Job _job;

  Focus _selected_field = Focus::Status;

  bool _edit_mode = false;
};

} // namespace tui
} // namespace kan

#endif
