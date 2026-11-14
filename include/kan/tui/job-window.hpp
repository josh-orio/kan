#ifndef TUI_JOB_WINDOW_HPP
#define TUI_JOB_WINDOW_HPP

#include <kan/core/job.hpp>
#include <kan/tui/comment-list.hpp>
#include <kan/tui/commit-list.hpp>
#include <kan/tui/history-viewer.hpp>
#include <kan/tui/issue-list.hpp>
#include <kan/tui/job-details.hpp>
#include <kan/tui/new-comment-window.hpp>
#include <kan/tui/tab-bar.hpp>
#include <termui/termui.hpp>

namespace kan {
namespace tui {

class JobWindow : termui::Interface {
  uint _width = 80, _height = 40;

  // tui::TabBar        _tab_bar = TabBar({"Details", "Comments", "History"});
  tui::TabBar        _tab_bar = TabBar({"Details", "Commits", "History"});
  tui::JobDetails    _job_detail;
  // tui::CommentList   _comment_list;
  tui::CommitList    _commit_list;
  tui::HistoryViewer _history_viewer;

  kan::Job _job;
  bool     _modified = false;

  bool _input_mode = false;

  void display();
  void process_input();

public:
  JobWindow(kan::Job job);

  std::pair<bool, kan::Job> show();
};

} // namespace tui
} // namespace kan

#endif
