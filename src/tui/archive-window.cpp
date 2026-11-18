#include <kan/tui.hpp>

namespace kan {
namespace tui {

void ArchiveWindow::display() {
  _width  = termui::terminal.width();
  _height = termui::terminal.height();

  termui::terminal.ClearScreen();
  termui::terminal.HideCursor();

  auto  content_w = _width - 4;
  auto &jobs      = board.archive;

  // title
  termui::terminal.MoveCursor(1, 2);
  termui::Text("Archive", termui::Styles::bold).render();
  termui::Text(std::format(" {} {}", jobs.size(), jobs.size() == 1 ? "job" : "jobs")).style(termui::Styles::faint).render();

  unsigned int r = 3;

  // one row per job with a blank row between, same as the commit list
  _visible_rows = _height > r + 1 ? (_height - r - 1 + 1) / 2 : 0;

  _cursor = jobs.empty() ? 0 : std::min<unsigned int>(_cursor, jobs.size() - 1);
  if (_cursor < _start_line) {
    _start_line = _cursor;
  }
  if (_visible_rows > 0 && _cursor >= _start_line + _visible_rows) {
    _start_line = _cursor - _visible_rows + 1;
  }

  if (jobs.empty()) {
    termui::terminal.MoveCursor(r, 2);
    termui::Text("Nothing archived. Archive a job with x in its job window.")
        .style(termui::Styles::faint)
        .align(termui::Text::Alignment::Center)
        .width(content_w)
        .height(1)
        .render();
  }

  for (unsigned int i = _start_line; i < std::min<size_t>(jobs.size(), _start_line + _visible_rows); i++) {
    auto &job   = jobs.at(i);
    bool  focus = i == _cursor;
    auto  row   = r + (2 * (i - _start_line));

    auto style       = focus ? board.styleMap.at("highlight") : termui::Style();
    auto bold_style  = termui::Style(style.foregroundColor, style.backgroundColor, {termui::SGR::Bold});
    auto faint_style = termui::Style(style.foregroundColor, style.backgroundColor, {termui::SGR::Faint});

    // cursor bar sits in the gutter, content starts 2 cells in (same as the other lists)
    if (focus) {
      termui::terminal.MoveCursor(row, 2);
      termui::Line(termui::Line::Direction::Vertical, 1).render();
    }

    // "#12  title ...  was in: Doing   archived <date>" - status + date pinned to the right
    auto right   = std::format("{}  {}", job.status, job.updated.toString());
    auto right_w = static_cast<unsigned int>(termui::visible_length(right));
    auto left_w  = (content_w - 2) > right_w + 1 ? (content_w - 2) - right_w - 1 : 0;

    termui::terminal.MoveCursor(row, 4);
    termui::TextRow(left_w).paddingStyle(style).add(termui::Text(std::format("#{}", job.id), faint_style)).add(termui::Text(job.title, bold_style)).render();

    termui::Text(" ", style).width(1).height(1).render();

    termui::TextRow(right_w).paddingStyle(style).add(termui::Text(job.status, faint_style)).add(termui::Text(job.updated.toString(), style)).render();
  }

  // control strip
  termui::terminal.MoveCursor(_height, 2);
  kan::tui::ControlStrip({
                             {"esc", "close"},
                             {"↑↓", "navigate"},
                             {"r  ↵", "restore"},
                             {"D", "delete"},
                         })
      .width(content_w)
      .render();

  termui::terminal.flush();
}

void ArchiveWindow::process_input() {
  auto  inp  = termui::terminal.read();
  auto &jobs = board.archive;

  if (inp == termui::key::ESC || inp == "q") {
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
  else if ((inp == "r" || inp == termui::key::ENTER) && !jobs.empty()) {
    board.beginUndo(std::format("restoring #{}", jobs.at(_cursor).id));
    board.restoreJob(jobs.at(_cursor)); // reloads, so the archive shrinks underneath us
    board.endUndo();
  }
  else if (inp == "D" && !jobs.empty()) {
    auto job = jobs.at(_cursor);

    if (DialogBox("Delete job", std::format("Delete #{} {} for good? u on the board brings it back until kan is closed.", job.id, job.title), "delete",
                  "cancel")
            .show()) {
      board.beginUndo(std::format("deleting #{}", job.id));
      board.deleteJob(job);
      board.endUndo();
    }
  }
}

void ArchiveWindow::show() {
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
