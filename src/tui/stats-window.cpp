#include <kan/tui.hpp>

namespace kan {
namespace tui {

void StatsWindow::display() {
  _width  = termui::terminal.width();
  _height = termui::terminal.height();

  termui::terminal.ClearScreen();
  termui::terminal.HideCursor();

  auto jobs   = board.allJobs();
  auto report = stats::build(board.Statuses(), jobs, std::time(nullptr), _weeks);

  auto content_w = _width - 4;

  // title
  termui::terminal.MoveCursor(1, 2);
  termui::Text("Stats", termui::Styles::bold).render();
  termui::Text(std::format(" last {} weeks", _weeks)).style(termui::Styles::faint).render();

  unsigned int r = 3;

  unsigned int label_w = 14;
  for (auto &c : report.columns) {
    label_w = std::max<unsigned int>(label_w, termui::visible_length(c.column) + 2);
  }

  auto line = [&](const std::string &label, const termui::string &value, termui::Style value_style = {}) {
    if (r >= _height - 1) {
      return;
    }
    termui::terminal.MoveCursor(r++, 4);
    termui::Text(label, termui::Styles::faint).width(label_w).height(1).render();
    termui::Text(value, value_style).width(content_w > label_w + 2 ? content_w - label_w - 2 : 0).height(1).render();
  };

  auto heading = [&](const std::string &text) {
    r++;
    if (r >= _height - 1) {
      return;
    }
    termui::terminal.MoveCursor(r++, 2);
    termui::Text(text, termui::Styles::bold).width(content_w).height(1).render();
  };

  // throughput - one bar per week, oldest on the left
  line("Finished", std::format("{} {}  ·  {} this week", report.finished, report.finished == 1 ? "job" : "jobs", report.finished_per_week.back()));
  line("", stats::sparkline(report.finished_per_week));
  line("Created", std::format("{} {}", report.created, report.created == 1 ? "job" : "jobs"));
  line("Cycle time", report.cycle_median_days
                         ? std::format("{} median  ·  85% within {}", stats::days(*report.cycle_median_days), stats::days(*report.cycle_p85_days))
                         : std::string("nothing finished yet"));

  // columns - jobs in each now, with a bar, and how long jobs stayed before moving on
  heading("Columns");

  int most = 1;
  for (auto &c : report.columns) {
    most = std::max(most, c.jobs_now);
  }

  for (auto &c : report.columns) {
    auto bar = std::string();
    for (int i = 0; i < (c.jobs_now * 20 + most - 1) / most; i++) {
      bar += "■";
    }

    line(c.column, std::format("{:<3} {:<21} {}", c.jobs_now, bar, c.average_days ? "stays " + stats::days(*c.average_days) : std::string()));
  }

  if (!report.finishers.empty()) {
    heading("Finished by");
    for (auto &f : report.finishers) {
      line(f.name, std::to_string(f.finished));
    }
  }

  if (!report.oldest.empty()) {
    heading("Waiting longest");
    for (auto &[job, d] : report.oldest) {
      line("#" + std::to_string(job->id), std::format("{} in {}  ·  {}", stats::days(d), job->status, job->title));
    }
  }

  // control strip
  termui::terminal.MoveCursor(_height, 2);
  kan::tui::ControlStrip({
                             {"esc", "close"},
                             {"←→", "fewer / more weeks"},
                         })
      .width(content_w)
      .render();

  termui::terminal.flush();
}

void StatsWindow::process_input() {
  auto inp = termui::terminal.read();

  if (inp == termui::key::ESC || inp == "q" || inp == "i") {
    state = State::Exit;
  }
  else if (inp == termui::key::L_ARROW) {
    _weeks = std::max(1, _weeks - 1);
  }
  else if (inp == termui::key::R_ARROW) {
    _weeks = std::min(52, _weeks + 1);
  }
}

void StatsWindow::show() {
  termui::terminal.ClearScreen();
  termui::terminal.ClearScrollback();

  state = State::Continue;

  do {
    display();
    process_input();
  } while (state == State::Continue);
}

} // namespace tui
} // namespace kan
