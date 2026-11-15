#include <kan/tui.hpp>

#include <format>

namespace kan {
namespace tui {

Board &Board::instance() {
  static Board board = Board();
  return board;
}

Board::Board() : _active_col(0), _active_row(0) {
    for (auto &c : board.columns) {
    _columns.push_back(tui::Column(c));
  }
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

  // onboarding - offer to add the current git identity to contributors.yaml
  auto me = User().getFromSystem();
  if (!me.name.empty() && !me.email.empty() && !board.isContributor(me)) {
    auto join = RegisterUser("Welcome", std::format("{} isn't a contributor to this board yet. Add yourself so you can be assigned jobs?", me.toString())).show();
    if (join) {
      board.addContributor(me);
    }
  }

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

  termui::Text(board.Title())
      .style(board.styleMap.at("homepage_title"))
      .height(1)
      .width(std::max<int>(_width - 4, termui::visible_length(board.Title())))
      .render();

    int cols       = board.Columns().size();

    int col_width  = ((_width - 2) / cols) - 1;
    int col_height = _height - 4; // 2 header, 2 footer

    termui::Line line = termui::Line(termui::Line::Direction::Vertical, col_height).style(board.styleMap.at("column_separator"));

    int row = 3, col = 2;

    for (int i = 0; i < board.Columns().size(); i++) {

      if (i == 0) {
        termui::terminal.MoveCursor(row, col);
        line.render();

        col++;
      }

      termui::terminal.MoveCursor(row, col);

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
  else if (inp == "n") {
    auto [success, new_job] = tui::NewJob().show();

    if (success) {
      new_job.status = board.Column(ActiveColumn()).title;
      new_job.id     = board.nextJobId();

      board.Column(ActiveColumn()).Jobs().push_back(new_job);

      board.saveJob(new_job);
    }

    state = termui::Interface::State::Continue;
  }
  else if (inp == "o" || inp == termui::key::ENTER) {

      // seralize job out to disk

      auto [modified, job] = tui::JobWindow(board.Job(ActiveColumn(), ActiveRow())).show();

      // if modified
      if (modified) {

        board.saveJob(job);
        board.reloadJobs();
      }

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
