#include <kan/tui.hpp>

namespace kan {
namespace tui {

JobWindow::JobWindow(kan::Job job) : _job(job), _job_detail(job), _comment_list(job.comments), _issue_list(job.issues), _history_viewer(job.history) {}

void JobWindow::display() {
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
    _job_detail.size(_width - 4, _height - 6).render();
  }
  else if (tab == 1) {
    _comment_list.size(_width - 4, _height - 6).render();
  }
  else if (tab == 2) {
    _issue_list.size(_width - 4, _height - 6).render();
  }
  else if (tab == 3) {
    _history_viewer.size(_width - 4, _height - 6).render();
  }

  termui::terminal.flush();
}

void JobWindow::process_input() {
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
    else if (tab == 2) {
      _issue_list.CursorUp();
    }
    else if (tab == 3) {
      _history_viewer.CursorUp();
    }
  }
  else if (inp == termui::key::D_ARROW) {
    if (tab == 0) {
      _job_detail.CursorDown();
    }
    else if (tab == 1) {
      _comment_list.CursorDown();
    }
    else if (tab == 2) {
      _issue_list.CursorDown();
    }
    else if (tab == 3) {
      _history_viewer.CursorDown();
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
        _job.comments.addComment(comment);
        _comment_list.ReloadComments(_job.comments);

        _job.history.addEvent({"Comment added.", User().getFromSystem(), Timestamp().Now()});
        _history_viewer.Reload(_job.history);

        _modified = true;
      }
    }
  }
  else if (inp == "i") {
    if (tab == 2) {
      auto [success, issue] = NewIssueWindow().show();

      // save issue
      

      if (success) {
        _job.issues.addIssue(issue);
        _issue_list.ReloadIssues(_job.issues);



        _job.history.addEvent({std::format("Issue added. Id: #{}", issue.uuid.substr(issue.uuid.size()-8)), User().getFromSystem(), Timestamp().Now()});
        _history_viewer.Reload(_job.history);

        _modified = true;
      }
    }
  }
  else if (inp == "o" || inp == termui::key::ENTER) {
    if (tab == 0) {
      if (_job_detail.Focussed() == JobDetails::Focus::Status) {
        std::vector<std::string> statuses = board.Statuses();

        auto [state, cursor] = tui::MiniMenu("Status", statuses).show();

        if (state == termui::Interface::State::Submit) {
          auto old_status = _job.status;
          _job.status     = statuses.at(cursor);

          _job.history.addEvent({std::format("Job status was updated: {} -> {}.", old_status, _job.status), User().getFromSystem(), Timestamp().Now()});
          _job_detail = JobDetails(_job);


          _modified   = true;
        }
      }

      if (_job_detail.Focussed() == JobDetails::Focus::Priority) {
        // std::vector<std::string> statuses = board.Statuses();
        std::vector<std::string> priorities = {"Low", "Normal", "High"};

        auto [state, cursor] = tui::MiniMenu("Priority", priorities).setCursor(1).show();

        if (state == termui::Interface::State::Submit) {
          _job.priority = Job::Priority(cursor);
          _job_detail   = JobDetails(_job);
          _modified     = true;
        }
      }

      else if (_job_detail.Focussed() == JobDetails::Focus::Description) {
        _job.description = EditWindow(_job.description).width(_width - 4).height(_height - 14).show();
        _job_detail      = JobDetails(_job);
        _modified        = true;
      }
      else if (_job_detail.Focussed() == JobDetails::Focus::Archive) {
      }
      else if (_job_detail.Focussed() == JobDetails::Focus::Delete) {
        auto confirm = DialogBox("Confirm delete", "Are you sure you want to delete this job?", "Delete", "Cancel").show();

        if (confirm) {
          board.deleteJob(_job);
          state = termui::Interface::State::Exit;
        }
      }
    }
    else if (tab == 1) {
      auto comment_view_state = ViewCommentWindow(_job.comments.at(_comment_list.GetCursor())).show();

      if (comment_view_state == termui::Interface::State::Delete) {
        _job.comments.deleteComment(_job.comments.at(_comment_list.GetCursor()));
        _comment_list.ReloadComments(_job.comments);
      }
    }

    else if (tab == 2) {
      auto issue_view_state = ViewIssueWindow(_job.issues.at(_issue_list.GetCursor())).show();

      if (issue_view_state == termui::Interface::State::Delete) {
        // _job.comments.deleteComment(_job.comments.at(_comment_list.GetCursor()));
        // _comment_list.ReloadComments(_job.comments);
      }
    }
  }
}

std::pair<bool, kan::Job> JobWindow::show() {
  state = State::Continue;

  do {
    display();
    process_input();

  } while (state == State::Continue);

  return {_modified, _job};
}

} // namespace tui
} // namespace kan
