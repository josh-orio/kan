#include <kan/tui.hpp>

namespace kan {
namespace tui {

JobWindow::JobWindow(kan::Job job) : _job(job), _job_detail(job), /*_comment_list(job.comments),*/ /*_issue_list(job.issues), */ _history_viewer(job.history) {}

void JobWindow::display() {
  // auto terminal_half_w = termui::terminal.halfWidth();
  // auto terminal_half_h = termui::terminal.halfHeight();

  // auto window_start_row = terminal_half_h - (_height / 2);
  // auto window_start_col = terminal_half_w - (_width / 2);

  _width  = termui::terminal.width();
  _height = termui::terminal.height();

  termui::terminal.ClearScreen();
  termui::terminal.HideCursor();

  auto tab = _tab_bar.GetTab();

  // title
  termui::terminal.MoveCursor(1, 2);
  termui::Text(_job.title, termui::Styles::bold).render();

  // job id
  termui::Text(" #" + std::to_string(_job.id)).style(termui::Styles::faint).render();

  // tab bar
  termui::terminal.MoveCursor(3, 2);
  // termui::Text(_job._title, termui::Styles::bold).width(_width - 4).height(1).render();
  _tab_bar.width(_width - 4).render();

  // clear content area
  termui::terminal.MoveCursor(5, 2);
  // termui::Text("").style(termui::Styles::none).width(_width - 4).height(_height - 6).render();
  // termui::Text("").style(termui::Style(termui::Color::Inherit(), termui::Colors::Red)).width(_width - 4).height(_height - 6).render();

  // termui::terminal.MoveCursor(window_start_row + 5, window_start_col + 2);

  // termui::terminal.flush(); // have to do this so child components can get an accurate cursor position

  if (tab == 0) {
    _job_detail.size(_width - 2, _height - 4).editMode(_input_mode).render();

    if (_input_mode) {
      termui::terminal.ShowCursor();
    }
  }
  else if (tab == 1) {
    // commits list
  }
  else if (tab == 2) {
    _history_viewer.size(_width - 4, _height - 4).render();
  }

  termui::terminal.flush();
}

void JobWindow::process_input() {
  auto inp = termui::terminal.read();

  auto tab = _tab_bar.GetTab();

  // if (_input_mode) {
  //   if (tab == 0) {
  //     // if (_job_detail.Focussed() == JobDetails::Focus::Description) {
  //     //   if ((std::string{32} <= inp) && (inp <= std::string{126})) { // accept basically all chars

  //     //     _job.description += inp;
  //     //   }
  //     //   else if (inp == termui::key::DEL) {
  //     //     _job.description = std::string(_job.description.begin(), _job.description.end() - 1);
  //     //   }
  //     //   else if (inp == termui::key::ENTER) {
  //     //     _job.description += "\n";
  //     //   }
  //     //   else if (inp == termui::key::ESC) {

  //     //     _job.addEvent({"Job description was changed.", User().getFromSystem(), Timestamp().Now()});

  //     //     // _job_detail     = JobDetails(_job);
  //     //     _history_viewer = HistoryViewer(_job.history);

  //     //     _modified   = true;
  //     //     _input_mode = false;
  //     //   }

  //     //   _job_detail.Reload(_job);
  //     // }
  //   }

  //   return;
  // }

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
      // _comment_list.CursorUp();
    }
    else if (tab == 2) {
      _history_viewer.CursorUp();
    }
  }
  else if (inp == termui::key::D_ARROW) {
    if (tab == 0) {
      _job_detail.CursorDown();
    }
    else if (tab == 1) {
      // _comment_list.CursorDown();
    }
    else if (tab == 2) {
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

  // else if (inp == "c") {
  //   if (tab == 1) {
  //     auto [success, comment] = NewCommentWindow().show();

  //     if (success) {
  //       _job.comments.addComment(comment);
  //       _comment_list.ReloadComments(_job.comments);

  //       _job.addEvent({"Comment added.", User().getFromSystem(), Timestamp().Now()});

  //       _job_detail = JobDetails(_job);
  //       _history_viewer.Reload(_job.history);
  //       _modified = true;
  //     }
  //   }
  // }

  // else if (inp == termui::key::ENTER) {
  //   if (tab == 0) {
  //     if (_job_detail.Focussed() == JobDetails::Focus::Status) {
  //       std::vector<std::string> statuses = board.Statuses();

  //       auto [state, cursor] = tui::MiniMenu("Status", statuses).show();

  //       if (state == termui::Interface::State::Submit) {
  //         auto old_status = _job.status;

  //         _job.status = statuses.at(cursor);

  //         _job.addEvent({std::format("Job status was updated: {} -> {}.", old_status, _job.status), User().getFromSystem(), Timestamp().Now()});

  //         _job_detail     = JobDetails(_job);
  //         _history_viewer = HistoryViewer(_job.history);
  //         _modified       = true;
  //       }
  //     }

  //     if (_job_detail.Focussed() == JobDetails::Focus::Priority) {
  //       std::vector<std::string> priorities = {"Low", "Normal", "High"};

  //       auto [state, cursor] = tui::MiniMenu("Priority", priorities).setCursor(1).show();

  //       if (state == termui::Interface::State::Submit) {
  //         auto old_priority_str = YAML::Node(_job.priority).as<std::string>();

  //         _job.priority = Job::Priority(cursor);

  //         auto new_priority_str = YAML::Node(_job.priority).as<std::string>();

  //         _job.addEvent({std::format("Job priority was updated: {} -> {}.", old_priority_str, new_priority_str), User().getFromSystem(), Timestamp().Now()});

  //         _job_detail     = JobDetails(_job);
  //         _history_viewer = HistoryViewer(_job.history);
  //         _modified       = true;
  //       }
  //     }
  //   }
  //   else if (tab == 1) {
  //     auto comment_view_state = ViewCommentWindow(_job.comments.at(_comment_list.GetCursor())).show();

  //     if (comment_view_state == termui::Interface::State::Delete) {
  //       _job.comments.deleteComment(_job.comments.at(_comment_list.GetCursor()));
  //       _comment_list.ReloadComments(_job.comments);
  //     }
  //   }
  // }

  else if (tab == 0) {
    if (inp == "s" || (inp == termui::key::ENTER && _job_detail.Focussed() == JobDetails::Focus::Status)) { // set status
      std::vector<std::string> statuses = board.Statuses();

      auto [state, cursor] = tui::MiniMenu("Status", statuses).show();

      if (state == termui::Interface::State::Submit) {
        auto old_status = _job.status;

        _job.status = statuses.at(cursor);

        _job.addEvent({std::format("Job status was updated: {} -> {}.", old_status, _job.status), User().getFromSystem(), Timestamp().Now()});

        _job_detail     = JobDetails(_job);
        _history_viewer = HistoryViewer(_job.history);
        _modified       = true;
      }
    }
    if (inp == "p" || (inp == termui::key::ENTER && _job_detail.Focussed() == JobDetails::Focus::Priority)) { // set priority
      std::vector<std::string> priorities = {"Low", "Normal", "High"};

      auto [state, cursor] = tui::MiniMenu("Priority", priorities).setCursor(1).show();

      if (state == termui::Interface::State::Submit) {
        auto old_priority_str = YAML::Node(_job.priority).as<std::string>();

        _job.priority = Job::Priority(cursor);

        auto new_priority_str = YAML::Node(_job.priority).as<std::string>();

        _job.addEvent({std::format("Job priority was updated: {} -> {}.", old_priority_str, new_priority_str), User().getFromSystem(), Timestamp().Now()});

        _job_detail     = JobDetails(_job);
        _history_viewer = HistoryViewer(_job.history);
        _modified       = true;
      }
    }

    if (inp == "a" || (inp == termui::key::ENTER && _job_detail.Focussed() == JobDetails::Focus::Assignee)) { // set assignee
      kan::tui::UserMenu userMenu;
      auto [state, selected_user] = userMenu.show();

      if (state == termui::Interface::State::Submit) {
        // std::cout << "Selected user: " << users[selected_user_index] << "\n";
        _job.assignee = selected_user;

        _job.addEvent({selected_user.Name() + " was set as assigned user.", User().getFromSystem(), Timestamp().Now()});

        _job_detail.Reload(_job);
        _history_viewer.Reload(_job.history);
        _modified = true;
      }
      else {
        std::cout << "User selection cancelled.\n";
      }
    }

    if (inp == "c") {
      auto [success, comment] = NewCommentWindow().show();

      if (success) {
        _job.comments.addComment(comment);
        // _comment_list.ReloadComments(_job.comments);

        _job.addEvent({"Comment added.", User().getFromSystem(), Timestamp().Now()});

        _job_detail.Reload(_job);
        _history_viewer.Reload(_job.history);
        _modified = true;
      }
    }

    else if (inp == "t") {
      _job_detail.GotoTop();
    }
    else if (inp == "b") {
      _job_detail.GotoBottom();
    }
  }

  else if (inp == "?") {
    JobWindowHelp().show();
  }
}

std::pair<bool, kan::Job> JobWindow::show() {
  termui::terminal.ClearScreen();
  termui::terminal.ClearScrollback();

  state = State::Continue;

  do {
    display();
    process_input();

  } while (state == State::Continue);

  return {_modified, _job};
}

} // namespace tui
} // namespace kan

// else if (_job_detail.Focussed() == JobDetails::Focus::Description) {
//   _input_mode=true;

// }

// auto [modified, description] = EditWindow(_job.description).width(_width - 4).height(_height - 14).show();
// _job_detail      = JobDetails(_job);
// _modified        = true;

//  if (state == termui::Interface::State::Submit) {
//   auto old_priority_str = YAML::Node(_job.priority).as<std::string>();

//   _job.priority = Job::Priority(cursor);

//   auto new_priority_str = YAML::Node(_job.priority).as<std::string>();

//   _job.addEvent({std::format("Job priority was updated: {} -> {}.", old_priority_str, new_priority_str), User().getFromSystem(), Timestamp().Now()});

//   _job_detail     = JobDetails(_job);
//   _history_viewer = HistoryViewer(_job.history);
//   _modified       = true;
// }
// else if (_job_detail.Focussed() == JobDetails::Focus::Archive) {
// }
// else if (_job_detail.Focussed() == JobDetails::Focus::Delete) {
//   auto confirm = DialogBox("Confirm delete", "Are you sure you want to delete this job?", "Delete", "Cancel").show();

//   if (confirm) {
//     board.deleteJob(_job);
//     state = termui::Interface::State::Exit;
//   }
// }

// }