#include <kan/tui.hpp>

#include <ctime>

namespace kan {
namespace tui {

// local calendar day, so "today" and "yesterday" follow the user's clock
static int dayNumber(time_t t) {
  std::tm tm = *std::localtime(&t);
  tm.tm_hour = 12; // noon dodges daylight saving edges
  tm.tm_min = tm.tm_sec = 0;
  return static_cast<int>(std::mktime(&tm) / 86400);
}

static std::string groupOf(time_t t) {
  auto days = dayNumber(std::time(nullptr)) - dayNumber(t);

  if (days <= 0) {
    return "Today";
  }
  if (days == 1) {
    return "Yesterday";
  }
  if (days < 7) {
    return "This week";
  }
  return "Earlier";
}

std::vector<kan::Job> FinishedWindow::matching() {
  std::vector<kan::Job> jobs;

  for (const auto &job : board.finished) {
    if (jobMatches(job, _query)) {
      jobs.push_back(job);
    }
  }

  return jobs;
}

void FinishedWindow::display() {
  _width  = termui::terminal.width();
  _height = termui::terminal.height();

  termui::terminal.ClearScreen();
  termui::terminal.HideCursor();

  auto content_w = _width - 4;
  auto jobs      = matching();

  _cursor = jobs.empty() ? 0 : std::min<unsigned int>(_cursor, jobs.size() - 1);

  // title + a little encouragement
  unsigned int this_week = 0;
  for (auto &job : board.finished) {
    this_week += groupOf(job.finished_at.time) != "Earlier";
  }

  termui::terminal.MoveCursor(1, 2);
  termui::Text("Finished", termui::Styles::bold).render();
  termui::Text(std::format(" {} {} this week", this_week, this_week == 1 ? "job" : "jobs")).style(termui::Styles::faint).render();

  // lay out every line: a header when the group changes, then one row + one blank per job
  struct Line {
    std::string header;
    int         job = -1;
  };
  std::vector<Line> lines;
  std::string       group;
  unsigned int      cursor_line = 0;

  for (int i = 0; i < static_cast<int>(jobs.size()); i++) {
    auto g = groupOf(jobs[i].finished_at.time);

    if (g != group) {
      if (!lines.empty()) {
        lines.push_back({});
      }
      lines.push_back({g});
      group = g;
    }

    if (i == static_cast<int>(_cursor)) {
      cursor_line = lines.size();
    }

    lines.push_back({"", i});
    lines.push_back({});
  }

  unsigned int r       = 3;
  unsigned int visible = _height > r + 2 ? _height - r - 2 : 0; // leave a gap above the control strip

  // keep the cursor row (and its group header when there's room) on screen
  if (cursor_line < _start_line) {
    _start_line = cursor_line > 0 && lines[cursor_line - 1].header.size() ? cursor_line - 1 : cursor_line;
  }
  if (visible > 0 && cursor_line >= _start_line + visible) {
    _start_line = cursor_line - visible + 1;
  }

  if (jobs.empty()) {
    termui::terminal.MoveCursor(r, 2);
    termui::Text(_query.empty() ? "Nothing finished yet. Press f on a job to strike it off." : "No finished jobs match.")
        .style(termui::Styles::faint)
        .align(termui::Text::Alignment::Center)
        .width(content_w)
        .height(1)
        .render();
  }

  for (unsigned int l = _start_line; l < std::min<size_t>(lines.size(), _start_line + visible); l++) {
    auto &line = lines[l];
    auto  row  = r + (l - _start_line);

    if (!line.header.empty()) {
      termui::terminal.MoveCursor(row, 2);
      termui::Text(line.header, termui::Styles::bold).width(content_w).height(1).render();
      continue;
    }

    if (line.job < 0) {
      continue;
    }

    auto &job   = jobs[line.job];
    bool  focus = line.job == static_cast<int>(_cursor);

    auto style        = focus ? board.styleMap.at("highlight") : termui::Style();
    auto struck_style = termui::Style(style.foregroundColor, style.backgroundColor, {termui::SGR::CrossedOut});
    auto faint_style  = termui::Style(style.foregroundColor, style.backgroundColor, {termui::SGR::Faint});

    // cursor bar sits in the gutter, content starts 2 cells in (same as the other lists)
    if (focus) {
      termui::terminal.MoveCursor(row, 2);
      termui::Line(termui::Line::Direction::Vertical, 1).render();
    }

    // "#12  ~~title~~  ...  Josh  3 commits  <date>" - who/commits/date pinned to the right
    auto commits = job.commits.size();
    auto right   = std::format("{}  {} {}  {}", job.finisher.Name(), commits, commits == 1 ? "commit" : "commits", job.finished_at.toString());
    auto right_w = static_cast<unsigned int>(termui::visible_length(right));
    auto left_w  = (content_w - 2) > right_w + 1 ? (content_w - 2) - right_w - 1 : 0;

    termui::terminal.MoveCursor(row, 4);
    termui::TextRow(left_w).paddingStyle(style).add(termui::Text(std::format("#{}", job.id), faint_style)).add(termui::Text(job.title, struck_style)).render();

    termui::Text(" ", style).width(1).height(1).render();

    termui::TextRow(right_w)
        .paddingStyle(style)
        .add(termui::Text(job.finisher.Name(), style))
        .add(termui::Text(std::format("{} {}", commits, commits == 1 ? "commit" : "commits"), faint_style))
        .add(termui::Text(job.finished_at.toString(), faint_style))
        .render();
  }

  // bottom row: search bar while searching or filtered, otherwise the control strip
  termui::terminal.MoveCursor(_height, 2);

  if (_searching || !_query.empty()) {
    termui::TextRow(content_w)
        .add(termui::Text("/", termui::Styles::bold))
        .add(termui::Text(_query.empty() ? std::string("search title, description or #number") : _query,
                          _query.empty() ? termui::Styles::faint : termui::Style()))
        .add(termui::Text(std::format("{} {}", jobs.size(), jobs.size() == 1 ? "match" : "matches"), termui::Styles::faint))
        .render();

    if (_searching) {
      termui::terminal.MoveCursor(_height, 2 + 2 + termui::visible_length(_query));
      termui::terminal.ShowCursor();
    }
  }
  else {
    kan::tui::ControlStrip({
                               {"esc", "close"},
                               {"↑↓", "navigate"},
                               {"↵", "open"},
                               {"u", "reopen"},
                               {"/", "search"},
                           })
        .width(content_w)
        .render();
  }

  termui::terminal.flush();
}

void FinishedWindow::process_input() {
  auto inp  = termui::terminal.read();
  auto jobs = matching();

  if (_searching) {
    if (inp == termui::key::ESC) {
      _searching = false;
      _query.clear();
    }
    else if (inp == termui::key::ENTER) {
      _searching = false;
    }
    else if (inp == termui::key::DEL) {
      if (!_query.empty()) {
        _query.pop_back();
      }
    }
    else if (inp == termui::key::U_ARROW) {
      _cursor = _cursor > 0 ? _cursor - 1 : 0;
    }
    else if (inp == termui::key::D_ARROW) {
      _cursor = _cursor + 1 < jobs.size() ? _cursor + 1 : _cursor;
    }
    else if (inp.size() == 1 && std::isprint(static_cast<unsigned char>(inp[0]))) {
      _query += inp;
      _cursor     = 0;
      _start_line = 0;
    }

    return;
  }

  if (inp == "/") {
    _searching = true;
  }
  else if (inp == termui::key::ESC && !_query.empty()) {
    _query.clear();
  }
  else if (inp == termui::key::ESC || inp == "q") {
    state = State::Exit;
  }
  else if (inp == termui::key::U_ARROW) {
    _cursor = _cursor > 0 ? _cursor - 1 : 0;
  }
  else if (inp == termui::key::D_ARROW) {
    _cursor = _cursor + 1 < jobs.size() ? _cursor + 1 : _cursor;
  }
  else if (inp == "t") {
    _cursor = 0;
  }
  else if (inp == "b") {
    _cursor = jobs.empty() ? 0 : jobs.size() - 1;
  }
  else if (inp == termui::key::ENTER && !jobs.empty()) {
    auto [modified, job] = JobWindow(jobs.at(_cursor)).show();

    if (modified) {
      board.saveJob(job);
      board.reloadJobs();
    }
  }
  else if (inp == "u" && !jobs.empty()) {
    auto job = jobs.at(_cursor);

    if (DialogBox("Reopen job",
                  std::format("Reopen #{} {}? It goes back to {}.", job.id, job.title, job.finished_from.empty() ? job.status : job.finished_from), "reopen",
                  "cancel")
            .show()) {
      board.reopenJob(job);
    }
  }
}

void FinishedWindow::show() {
  termui::terminal.ClearScreen();
  termui::terminal.ClearScrollback();

  state = State::Continue;

  do {
    display();
    process_input();

  } while (state == State::Continue);

  termui::terminal.ClearScreen();
}

} // namespace tui
} // namespace kan
