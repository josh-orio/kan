#include <kan/tui.hpp>

namespace kan {
namespace tui {

JobWindow::JobWindow(kan::Job job) : _job(job), _job_detail(job), _commit_list(job.commits), _history_viewer(job.history) {}

void JobWindow::display() {
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
  _tab_bar.width(_width - 4).render();

  // clear content area
  termui::terminal.MoveCursor(5, 2);

  if (tab == 0) {
    _job_detail.size(_width - 2, _height - 4).editMode(_input_mode).render();

    if (_input_mode) {
      termui::terminal.ShowCursor();
    }
  }
  else if (tab == 1) {
    termui::terminal.MoveCursor(5, 2);
    _commit_list.size(_width - 4, _height - 4).render();
  }
  else if (tab == 2) {
    _history_viewer.size(_width - 4, _height - 4).render();
  }

  termui::terminal.flush();
}

void JobWindow::process_input() {
  auto inp = termui::terminal.read();

  auto tab = _tab_bar.GetTab();

  if (inp == "?") { // checked first so it works on every tab
    HelpDialog(jobWindowHelp).show();
  }
  else if (inp == termui::key::ESC || inp == "q") {
    state = termui::Interface::State::Exit;
  }
  else if (inp == "g") {
    switchToBranch();
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
      _commit_list.CursorUp();
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
      _commit_list.CursorDown();
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

        if (board.finish_on_last_column && _job.status == statuses.back()) {
          board.finishJob(_job);
          _modified = false; // already saved
          state     = termui::Interface::State::Exit;
          return;
        }
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
      auto [state, selected_user] = userMenu.setCursor(_job.assignee).show();

      if (state == termui::Interface::State::Submit) {
        auto old_assignee = _job.assignee.name.empty() ? std::string("None") : _job.assignee.Name();

        _job.assignee = selected_user;

        _job.addEvent({std::format("Job assignee was updated: {} -> {}.", old_assignee, _job.assignee.Name()), User().getFromSystem(), Timestamp().Now()});

        _job_detail.Reload(_job);
        _history_viewer.Reload(_job.history);
        _modified = true;
      }
    }

    if (inp == "e" || (inp == termui::key::ENTER && _job_detail.DescriptionSelected())) { // edit title + description
      auto [saved, edited] = NewJob(_job).show();

      if (saved) {
        if (edited.title != _job.title) {
          _job.addEvent({std::format("Job title was updated: {} -> {}.", _job.title, edited.title), User().getFromSystem(), Timestamp().Now()});
          _job.title = edited.title;
          _modified  = true;
        }

        if (edited.description != _job.description) {
          _job.description = edited.description;
          _job.addEvent({"Job description was updated.", User().getFromSystem(), Timestamp().Now()});
          _modified = true;
        }

        _job_detail.Reload(_job);
        _history_viewer.Reload(_job.history);
      }
    }

    if (inp == "f" && !_job.finished) {
      if (DialogBox("Finish job", std::format("Finish #{} {}? It moves to the finished log (F on the board).", _job.id, _job.title), "finish", "cancel")
              .show()) {
        board.finishJob(_job); // saves any other edits made in this window too
        _modified = false;
        state     = termui::Interface::State::Exit;
        return;
      }
    }

    if (inp == "x") {
      if (DialogBox("Archive job", std::format("Archive #{} {}? You can restore it from the archive (a) on the board.", _job.id, _job.title), "archive",
                    "cancel")
              .show()) {
        board.archiveJob(_job); // saves any other edits made in this window too
        _modified = false;      // already saved - stop the board saving it back onto the board
        state     = termui::Interface::State::Exit;
        return;
      }
    }

    if (inp == "D") {
      if (DialogBox("Delete job",
                    std::format("Delete #{} {} for good? This can't be undone - archive it with x if you might want it back.", _job.id, _job.title), "delete",
                    "cancel")
              .show()) {
        board.deleteJob(_job);
        _modified = false;
        state     = termui::Interface::State::Exit;
        return;
      }
    }

    if (inp == "c") {
      auto [success, comment] = NewCommentWindow().show();

      if (success) {
        _job.comments.addComment(comment);

        _job.addEvent({"Comment added.", User().getFromSystem(), Timestamp().Now()}, false);

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

  else if (tab == 1) {
    if ((inp == termui::key::ENTER || inp == "o") && !_commit_list.Empty()) {
      CommitWindow(_commit_list.Selected()).show();
    }
    else if (inp == "t") {
      _commit_list.GotoTop();
    }
    else if (inp == "b") {
      _commit_list.GotoBottom();
    }
  }
}

// g - check out the job's branch, making it first if there isn't one
void JobWindow::switchToBranch() {
  auto branch = board.branchFor(_job.id);

  if (branch && branch->current) {
    DialogBox::notice("Branch", std::format("You're already on {}.", branch->name));
    return;
  }

  auto name   = branch ? branch->name : git::jobBranchName(_job.id, _job.title);
  auto create = branch == nullptr;

  auto question = create ? std::format("Create the branch {} and switch to it?", name) : std::format("Switch to the branch {}?", name);

  if (!DialogBox("Branch", question, "switch", "cancel").show()) {
    return;
  }

  if (auto error = git::switchBranch(name, create); !error.empty()) {
    DialogBox::notice("Couldn't switch branch", error);
    return;
  }

  if (create) {
    _job.addEvent({std::format("Branch {} created.", name), User().getFromSystem(), Timestamp().Now()});
    _history_viewer.Reload(_job.history);
    _modified = true;
  }

  board.loadBranches();
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
