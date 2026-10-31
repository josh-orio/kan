#include <kan/tui.hpp>

#include <format>

namespace kan {
namespace tui {

Board &Board::instance() {
  static Board board = Board();
  return board;
}

Board::Board() : _active_col(0), _active_row(0) {
    for (const auto &c : board.columns) {
    _columns.push_back(tui::Column(c));
  }
}

void Board::load() {
  // for (const auto &c : board.columns) {
  //   _columns.push_back(tui::Column(c));
  // }
}

Board &Board::width(uint n) {
  _width = n;
  return *this;
}

Board &Board::height(uint n) {
  _height = n;
  return *this;
}

uint Board::ActiveColumn() { return _active_col; }

uint Board::ActiveRow() { return _active_row; }

void Board::Up(uint n) {
  _active_row = std::max<int>(0, _active_row - n);
  _columns.at(_active_col).cursorUp();
}

void Board::Down(uint n) {
  _active_row = std::min<int>(board.Jobs(_active_col).size() == 0 ? 0 : board.Jobs(_active_col).size() - 1, _active_row + n);
  _columns.at(_active_col).cursorDown();
}

void Board::Left(uint n) {
  _active_col = std::max<int>(0, _active_col - n);

  if (_active_row > board.Jobs(_active_col).size()) {
    _active_row = board.Jobs(_active_col).size();

  }
}

void Board::Right(uint n) {
  _active_col = std::min<int>(board.Columns().size() - 1, _active_col + n);

  if (_active_row > board.Jobs(_active_col).size()) {
    _active_row = board.Jobs(_active_col).size();
  }
}

Board &Board::ActiveColumn(uint col) {
  _active_col = col;
  return *this;
}

Board &Board::ActiveRow(uint row) {
  _active_row = row;
  return *this;
}

void Board::show() {
  termui::TermSetups::fullscreen_no_mouse.configure();

  state = State::Continue;

  do {
    display();
    process_input();

  } while (state == State::Continue);

  termui::TermSetups::fullscreen_no_mouse.reset();
}

void Board::display() {
  internal_update();

  termui::terminal.ClearScreen();
  termui::terminal.ClearScrollback();

  termui::terminal.MoveCursor(1, 2);

  // auto ts = board.styleMap.at("debug");
  // auto tsm = board.styleMap.at("main");

  // termui::Text(board.Title()).style(board.styleMap.at("mainpage_title")).height(1).width(_width).render();
  termui::Text(board.Title())
      .style(board.styleMap.at("homepage_title"))
      .height(1)
      .width(std::max<int>(_width - 4, termui::visible_length(board.Title())))
      .render();

  // termui::terminal.MoveCursor(3, 3);
  // _tab_bar.width(_width - 4).render();

  // termui::terminal.MoveCursor(5, 2);
  // termui::terminal.flush(); // have to do this so child components can get an accurate cursor position

  // auto tab = _tab_bar.GetTab();



  // if (tab == 0) {
    // int w          = _width;
    int cols       = board.Columns().size();
    // int col_width  = _width <= 0 ? 0 : [w, cols]() {
    //   int overhead    = 4;
    //   int col_spacing = 1;

    //   return ((w - overhead) / cols) - col_spacing;
    // }();
    int col_width  = ((_width - 1) / cols) - 1;
    int col_height = _height - 4; // 2 header, 2 footer

    auto color = termui::terminal.BackgroundColor();

    int tamper = 60 * (termui::terminal.HasDarkBackground() ? 1 : -1);

    color.rgb.r += tamper;
    color.rgb.g += tamper;
    color.rgb.b += tamper;

    termui::Line line = termui::Line(termui::Line::Direction::Vertical, col_height).style(termui::Style(color, termui::Color()));

    int row = 3, col = 2;

    for (int i = 0; i < board.Columns().size(); i++) {

      if (i == 0) {
        termui::terminal.MoveCursor(row, col);
        line.render();

        col++;
      }

      termui::terminal.MoveCursor(row, col);
      // termui::terminal.flush();

      // kan::tui::Column column(board.Column(i));
      auto& column = _columns.at(i);

      if (i == _active_col) {
        column.focus();

        if (i == _active_col) {
          column.setCursor(_active_row);
        }
      }
      else {
        column.blur();
      }

      column.width(col_width).height(col_height);
      column.render();

      col += col_width;

      termui::terminal.MoveCursor(row, col);
      line.render();

      col++;
    }

    termui::terminal.MoveCursor(termui::terminal.height(), 2);

    ControlStrip({
                     {"←↑↓→", "navigate"},
                     {"tab", "section"},
                     {"o", "open"},
                     {"n", "new job"},
                     {"?", "help"},
                     {"s", "settings"},
                     {"q", "quit"},
                 })
        .width(_width - 2)
        .render();
  // }
  // else if (tab == 1) {
  //   // issue_list.size(_width - 4, _height - 6).render();

  //   // termui::terminal.MoveCursor(termui::terminal.height(), 2);

  //   // ControlStrip({
  //   //                  {"←↑↓→", "navigate"},
  //   //                  {"tab", "section"},
  //   //                  {"o", "open"},
  //   //                  {"i", "new issue"},
  //   //                  {"?", "help"},
  //   //                  {"s", "settings"},
  //   //                  {"q", "quit"},
  //   //              })
  //   //     .width(_width - 2)
  //   //     .render();
  // }

  // termui::terminal.MoveCursor(termui::terminal.height(), 2);

  // ControlStrip({
  //                  {"←↑↓→", "navigate"},
  //                  {"tab", "section"},
  //                  {"o", "open"},
  //                  {"n", "new job"},
  //                  {"?", "help"},
  //                  {"s", "settings"},
  //                  {"q", "quit"},
  //              })
  //     .width(_width - 2)
  //     .render();

  termui::terminal.flush();
}

void Board::process_input() {
  std::string inp = termui::terminal.read();

  // auto tab = _tab_bar.GetTab();

  if (inp == "q") {
    state = termui::Interface::State::Exit;
    // serialize board out to disk
  }
  else if (inp == termui::key::U_ARROW) {
    // if (tab == 0) {
      Up();
    // }
    // else if (tab == 1) {
    //   // issue_list.CursorUp();
    // }
  }
  else if (inp == termui::key::D_ARROW) {
    // if (tab == 0) {
      Down();
    // }
    // else if (tab == 1) {
    //   // issue_list.CursorDown();
    // }
  }
  else if (inp == termui::key::L_ARROW) {
    // if (tab == 0) {
      Left();
    // }
  }
  else if (inp == termui::key::R_ARROW) {
    // if (tab == 0) {
      Right();
    // }
  }
  // else if (inp == termui::key::Tab) {
  //   _tab_bar.TabIncrement();
  // }
  // else if (inp == termui::key::ShiftTab) {
  //   _tab_bar.TabDecrement();
  // }
  else if (inp == "n") {
    auto [success, new_job] = tui::NewJob().show();

    if (success) {
      new_job.status = board.Column(ActiveColumn()).title; //.Title(); // board._columns.at(_renderer.ActiveColumn())._title;

      board.Column(ActiveColumn()).Jobs().push_back(new_job); // board._columns.at(_renderer.ActiveColumn())._jobs.push_back(new_job);

      board.saveJob(new_job);
    }

    state = termui::Interface::State::Continue;
  }
  else if (inp == "o" || inp == termui::key::ENTER) {

    // if (tab == 0) {
      // seralize job out to disk

      auto [modified, job] = tui::JobWindow(board.Job(ActiveColumn(), ActiveRow())).show();

      // if modified
      if (modified) {
        // job.modifier = User().getFromSystem();
        // job.updated  = Timestamp().Now();

        // job.modifier = job.history.events.back().user;
        // job.updated = job.history.events.back().time;

        board.saveJob(job);
        board.reloadJobs();
      }
    // }
    // else if (tab == 1) {
    //   // auto issue_view_state = ViewIssueWindow(board.issues.at(issue_list.GetCursor())).show();

    //   // if (issue_view_state == termui::Interface::State::Delete) {
    //     // _job.comments.deleteComment(_job.comments.at(_comment_list.GetCursor()));
    //     // _comment_list.ReloadComments(_job.comments);
    //   // }
    // }
  }
  // else if (inp == "i") {
  //   if (tab == 1) {
  //     auto [success, issue] = NewIssueWindow().show();

  //     if (success) {
  //       board.saveIssue(issue);
  //       board.issues.addIssue(issue);
  //       // issue_list.ReloadIssues(board.issues);
  //       // issue_list.ReloadIssues();

  //       // register with board history?

  //       // _job.history.addEvent({std::format("Issue added. Id: #{}", issue.uuid.substr(issue.uuid.size()-8)), User().getFromSystem(), Timestamp().Now()});
  //       // _history_viewer.Reload(_job.history);

  //       // _modified = true;
  //     }

  //     //   auto [success, new_job] = tui::NewJob().show();

  //     // if (success) {
  //     //   new_job.status = board.Column(ActiveColumn()).title; //.Title(); // board._columns.at(_renderer.ActiveColumn())._title;

  //     //   board.Column(ActiveColumn()).Jobs().push_back(new_job); // board._columns.at(_renderer.ActiveColumn())._jobs.push_back(new_job);

  //     //   board.saveJob(new_job);
  //     // }
  //   }
  // }
  else if (inp == "s") {
    tui::SettingsDialog().show();
  }

  else if (inp == "?") {
    tui::HelpDialog().show();
  }
}

void Board::internal_update() {
  _height = termui::terminal.height();
  _width  = termui::terminal.width();

  // issue_list.ReloadIssues();
}

} // namespace tui
} // namespace kan
