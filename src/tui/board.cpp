#include <kan/tui.hpp>

#include <format>

namespace kan {
namespace tui {

Board::Board(kan::Board &board) : /* _board(board), */ _active_col(0), _active_row(0) {}

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

void Board::Up(uint n) { _active_row = std::max<int>(0, _active_row - n); }

void Board::Down(uint n) { _active_row = std::min<int>(board.Jobs(_active_col).size() == 0 ? 0 : board.Jobs(_active_col).size() - 1, _active_row + n); }

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

// void Board::display() {
//   internal_update();

//   _renderer.render();

//   termui::terminal.flush();
// }


void Board::display() {
  internal_update();

    termui::terminal.ClearScreen();
  termui::terminal.ClearScrollback();

  termui::terminal.MoveCursor(1, 2);
  termui::Text(board.Title(), termui::Styles::bold).height(1).width(20).render();

  termui::terminal.MoveCursor(3, 3);
  _tab_bar.width(_width - 4).render();

  termui::terminal.MoveCursor(5, 2);
  termui::terminal.flush(); // have to do this so child components can get an accurate cursor position

  auto tab = _tab_bar.GetTab();

  if (tab == 0) {
    int w          = _width;
    int cols       = board.Columns().size();
    int col_width  = _width <= 0 ? 0 : [w, cols]() {
      int overhead    = 4;
      int col_spacing = 1;

      return ((w - overhead) / cols) - col_spacing;
    }();
    int col_height = _height - 6; // 2 header, 2 tab bar, 2 footer

    for (int i = 0; i < board.Columns().size(); i++) {

      termui::terminal.MoveCursor(5, 2 + (i * col_width));
      termui::terminal.flush();

      kan::tui::Column column(board.Column(i));

      if (i == _active_col) {
        if (_active_row > board.Jobs(i).size()) {
          _active_row = board.Jobs(i).size();
        }

        column.focus(_active_row);
      }
      else {
        column.blur();
      }

      column.width(col_width).height(col_height);
      column.render();
    }
  }
  else if (tab == 1) {
    // issue llist
    
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

  termui::terminal.flush();
}

void Board::process_input() {
  std::string inp = termui::terminal.read();

  if (inp == "q") {
    state = termui::Interface::State::Exit;
    // serialize board out to disk
  }
  else if (inp == termui::key::U_ARROW) {
    Up();
  }
  else if (inp == termui::key::D_ARROW) {
    Down();
  }
  else if (inp == termui::key::L_ARROW) {
    Left();
  }
  else if (inp == termui::key::R_ARROW) {
    Right();
  }
  else if (inp == termui::key::Tab) {
    _tab_bar.TabIncrement();
  }
  else if (inp == termui::key::ShiftTab) {
    _tab_bar.TabDecrement();
  }
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

    // seralize job out to disk

    auto [modified, job] = tui::JobWindow(board.Job(ActiveColumn(), ActiveRow())).show();

    // if modified
    if (modified) {
      job.modifier = User().getFromSystem();
      job.updated = Timestamp().Now();
     board. saveJob(job);
     board. reloadJobs();
    }

    state = termui::Interface::State::Continue;
  }
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
}

} // namespace tui
} // namespace kan
