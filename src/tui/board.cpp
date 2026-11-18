#include <kan/tui.hpp>

#include <algorithm>
#include <format>
#include <sstream>

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

static std::string lower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
  return s;
}

// every word in the query has to match - "#12" or "12" matches job number 12, anything else is a
// case-insensitive substring of the title or description
bool jobMatches(const kan::Job &job, const std::string &query) {
  auto text = lower(job.title + "\n" + job.description);

  std::istringstream words(lower(query));
  std::string        word;

  while (words >> word) {
    auto number = word.starts_with("#") ? word.substr(1) : word;
    bool is_id  = !number.empty() && number.find_first_not_of("0123456789") == std::string::npos && std::to_string(job.id) == number;

    if (!is_id && text.find(word) == std::string::npos) {
      return false;
    }
  }

  return true;
}

std::vector<unsigned int> Board::visibleRows(unsigned int col) {
  std::vector<unsigned int> rows;
  auto                     &jobs = board.Jobs(col);

  for (unsigned int i = 0; i < jobs.size(); i++) {
    if (jobMatches(jobs[i], _query)) {
      rows.push_back(i);
    }
  }

  return rows;
}

void Board::setQuery(std::string query) {
  _query      = query;
  _active_row = 0;
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
  auto count  = visibleRows(_active_col).size();
  _active_row = std::min<int>(count == 0 ? 0 : count - 1, _active_row + n);
  _columns.at(_active_col).cursorDown();
}

void Board::Left(uint n) {
  _active_col = std::max<int>(0, _active_col - n);

  if (_active_row > visibleRows(_active_col).size()) {
    _active_row = visibleRows(_active_col).size();
  }
}

void Board::Right(uint n) {
  _active_col = std::min<int>(board.Columns().size() - 1, _active_col + n);

  if (_active_row > visibleRows(_active_col).size()) {
    _active_row = visibleRows(_active_col).size();
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
    auto join = DialogBox("Welcome", std::format("{} isn't a contributor to this board yet. Add yourself so you can be assigned jobs?", me.toString())).show();
    if (join) {
      board.addContributor(me);
    }
  }

  auto dir   = board.Path();
  _kan_watch = kan::Watcher({dir / "kan.yaml", dir / "jobs", dir / "contributors.yaml"});

  if (auto git_dir = git::gitDir(); !git_dir.empty()) {
    auto common = git::commonDir();
    _git_watch  = kan::Watcher({git_dir / "HEAD", git_dir / "logs" / "HEAD", common / "packed-refs", common / "refs" / "heads"});
  }

  state = State::Continue;

  do {
    if (_redraw) {
      display();
    }
    _redraw = true;

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

  int cols = board.Columns().size();

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

    auto &column = _columns.at(i);

    column.filter(visibleRows(i));

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

  if (_searching || !_query.empty()) {
    // search bar replaces the control strip: "/ query  3 matches          esc clear  ↵ done"
    unsigned int count = 0;
    for (unsigned int i = 0; i < board.Columns().size(); i++) {
      count += visibleRows(i).size();
    }

    auto strip   = _searching ? ControlStrip({{"esc", "clear"}, {"↑↓", "navigate"}, {"↵", "done"}})
                              : ControlStrip({{"esc", "clear"}, {"/", "edit search"}, {"o", "open"}});
    auto strip_w = 34u;
    auto bar_w   = _width - 2 > strip_w + 1 ? _width - 2 - strip_w - 1 : 0;

    termui::TextRow(bar_w)
        .add(termui::Text("/", termui::Styles::bold))
        .add(termui::Text(_query.empty() ? std::string("search title, description or #number") : _query,
                          _query.empty() ? termui::Styles::faint : termui::Style()))
        .add(termui::Text(std::format("{} {}", count, count == 1 ? "match" : "matches"), termui::Styles::faint))
        .render();

    termui::terminal.MoveCursor(termui::terminal.height(), 2 + bar_w + 1);
    strip.width(strip_w).render();

    if (_searching) { // park the terminal cursor at the end of the query
      termui::terminal.MoveCursor(termui::terminal.height(), 2 + 2 + termui::visible_length(_query));
      termui::terminal.ShowCursor();
    }
    else {
      termui::terminal.HideCursor();
    }
  }
  else {
    termui::terminal.HideCursor();

    ControlStrip({
                     {"←↑↓→", "navigate"},
                     {"/", "search"},
                     {"o", "open"},
                     {"n", "new job"},
                     {"f", "finish"},
                     {"u", "undo"},
                     {"F", "finished"},
                     {"a", "archive"},
                     {"?", "help"},
                     {"s", "settings"},
                     {"q", "quit"},
                 })
        .width(_width - 2)
        .render();
  }

  termui::terminal.flush();
}

void Board::rebuildColumns() { // the tui columns point into the old ones
  _columns.clear();
  for (auto &c : board.columns) {
    _columns.push_back(tui::Column(c));
  }

  _active_col = std::min<uint>(_active_col, board.columns.empty() ? 0 : board.columns.size() - 1);

  auto rows   = visibleRows(_active_col);
  _active_row = std::min<uint>(_active_row, rows.empty() ? 0 : rows.size() - 1);
}

bool Board::refresh() {
  bool changed = termui::terminal.width() != _width || termui::terminal.height() != _height; // resized

  if (_git_watch.changed()) {
    board.loadBranches();
    changed = true;
  }

  if (!_kan_watch.changed()) {
    return changed;
  }

  // keep the cursor on the same job if it's still in this column
  auto        rows     = visibleRows(_active_col);
  std::string selected = _active_row < rows.size() ? board.Job(_active_col, rows[_active_row]).uuid : "";

  try {
    if (board.refresh()) {
      rebuildColumns();
    }
  }
  catch (const std::exception &e) { // a file caught half written, mid pull say - have another go next time
    logMessage(std::string("refresh failed: ") + e.what());
    _kan_watch.reset();
  }

  if (board.columns.empty()) {
    return true;
  }

  rows        = visibleRows(_active_col);
  _active_row = std::min<uint>(_active_row, rows.empty() ? 0 : rows.size() - 1);

  for (uint i = 0; i < rows.size(); i++) {
    if (board.Job(_active_col, rows[i]).uuid == selected) {
      _active_row = i;
    }
  }

  return true;
}

void Board::process_input() {
  std::string inp = readKey(1000);

  if (inp.empty()) { // nothing pressed, see if anything changed under us
    _redraw = refresh();
    return;
  }

  // while typing a search, keys go into the query (arrows still move through the results)
  if (_searching) {
    if (inp == termui::key::ESC) {
      _searching = false;
      setQuery("");
    }
    else if (inp == termui::key::ENTER) {
      _searching = false;
    }
    else if (inp == termui::key::DEL) {
      if (!_query.empty()) {
        setQuery(_query.substr(0, _query.size() - 1));
      }
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
    else if (inp.size() == 1 && std::isprint(static_cast<unsigned char>(inp[0]))) {
      setQuery(_query + inp);
    }

    return;
  }

  if (inp == "/") {
    _searching = true;
  }
  else if (inp == termui::key::ESC && !_query.empty()) {
    setQuery("");
  }
  else if (inp == "q") {
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

      board.beginUndo(std::format("new job #{}", new_job.id));
      board.saveJob(new_job);
      board.endUndo();
    }

    state = termui::Interface::State::Continue;
  }
  else if (inp == "o" || inp == termui::key::ENTER) {

    // seralize job out to disk

    auto rows = visibleRows(ActiveColumn());

    if (ActiveRow() >= rows.size()) {
      return;
    }

    auto opened = board.Job(ActiveColumn(), rows.at(ActiveRow()));

    board.beginUndo(std::format("changes to #{}", opened.id)); // finishing, archiving or deleting from the window are saved in there
    auto [modified, job] = tui::JobWindow(opened).show();

    if (modified) {
      board.saveJob(job);
      board.reloadJobs();
    }
    board.endUndo();
  }

  else if (inp == "s") {
    tui::SettingsDialog().show();
    rebuildColumns();
  }

  else if (inp == "a") {
    tui::ArchiveWindow().show();
  }

  else if (inp == "F") {
    tui::FinishedWindow().show();
  }

  else if (inp == "f") { // strike off the selected job, or un-strike it
    auto rows = visibleRows(ActiveColumn());

    if (ActiveRow() < rows.size()) {
      auto job = board.Job(ActiveColumn(), rows.at(ActiveRow()));

      if (job.finished) {
        board.beginUndo(std::format("reopening #{}", job.id));
        board.reopenJob(job);
        board.endUndo();
      }
      else if (DialogBox("Finish job", std::format("Finish #{} {}? It moves to the finished log (F).", job.id, job.title), "finish", "cancel").show()) {
        board.beginUndo(std::format("finishing #{}", job.id));
        board.finishJob(job);
        board.endUndo();
      }
    }
  }

  else if (inp == "u") {
    if (board.canUndo()) {
      auto result = board.undo();
      rebuildColumns();
      DialogBox::notice("Undo", result);
    }
    else {
      DialogBox::notice("Undo", "Nothing to undo yet. Undo covers changes to jobs made since kan was opened.");
    }
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
