#include <kan/tui.hpp>

namespace kan {
namespace tui {

const std::vector<HelpSection> boardHelp = {
    {"Board",
     {
         {"←↑↓→", "move between jobs"},
         {"o  ↵", "open the selected job"},
         {"n", "new job in this column"},
         {"f", "finish (strike off) the job, again to undo"},
         {"u", "undo the last change to a job"},
         {"F", "finished jobs"},
         {"a", "archived jobs"},
         {"i", "stats: throughput, cycle time, time in each column"},
         {"s", "settings: columns and their styles"},
         {"q", "quit"},
     }},
    {"Search",
     {
         {"/", "search titles, descriptions and #numbers"},
         {"↵", "keep the results, stop typing"},
         {"esc", "clear the search"},
     }},
    {"Git",
     {
         {"", "Mention #12 in a commit message to link that"},
         {"", "commit to job 12 - it shows up in the job's"},
         {"", "Commits tab. \"fixes #12\" (or closes, finishes,"},
         {"", "resolves) finishes the job too."},
         {"⎇", "the job has a branch, ⎇✓ it's been merged"},
     }},
    {"Command line",
     {
         {"", "kan new, ls, show, mv, assign, finish and more"},
         {"", "work from the shell too - kan help lists them."},
     }},
};

const std::vector<HelpSection> jobWindowHelp = {
    {"Job",
     {
         {"tab", "next tab"},
         {"↑↓", "navigate"},
         {"esc", "close the job"},
     }},
    {"Details",
     {
         {"e", "edit title and description"},
         {"s", "set status"},
         {"p", "set priority"},
         {"a", "set assignee"},
         {"c", "add a comment"},
         {"↵", "read a comment in full, or open a commit"},
         {"t  b", "jump to top / bottom"},
     }},
    {"Finish, archive & delete",
     {
         {"f", "finish the job"},
         {"x", "archive the job"},
         {"D", "delete the job for good"},
     }},
    {"Commits",
     {
         {"↵", "view commit, then ↵ on a file for its diff"},
         {"s", "toggle side by side diff"},
     }},
    {"Git",
     {
         {"g", "switch to the job's branch, creating it"},
         {"", "(12-fix-login-crash) if it doesn't exist"},
     }},
};

HelpDialog::HelpDialog(std::vector<HelpSection> sections) : _sections(sections) {}

void HelpDialog::display() {
  auto key_w = 6u;

  // fit the box to the content: title + blank, then each section's header, rows and a blank line, then the control strip
  _width  = 56;
  _height = 3 + 2;
  for (const auto &section : _sections) {
    _height += 1 + section.rows.size() + 1;
  }

  auto terminal_half_w = termui::terminal.halfWidth();
  auto terminal_half_h = termui::terminal.halfHeight();

  auto window_start_row = terminal_half_h - (_height / 2);
  auto window_start_col = terminal_half_w - (_width / 2);

  // clear the area behind the box
  for (unsigned int r = 0; r < _height; r++) {
    termui::terminal.MoveCursor(window_start_row + r, window_start_col);
    termui::Text("").width(_width).height(1).render();
  }

  // box
  termui::terminal.MoveCursor(window_start_row, window_start_col);
  termui::Box(_width, _height, termui::Borders::rounded, termui::Styles::faint).render();

  // title
  termui::terminal.MoveCursor(window_start_row + 1, window_start_col + 2);
  termui::Text("KAN I Help You?", termui::Styles::bold).width(_width - 4).height(1).render();

  auto row = window_start_row + 3;

  for (const auto &section : _sections) {
    termui::terminal.MoveCursor(row++, window_start_col + 2);
    termui::Text(section.title, termui::Styles::bold).width(_width - 4).height(1).render();

    for (const auto &[key, description] : section.rows) {
      if (key.empty()) {
        termui::terminal.MoveCursor(row++, window_start_col + 4);
        termui::Text(description, termui::Styles::faint).width(_width - 6).height(1).render();
        continue;
      }

      // keys are right aligned in their own column so the descriptions line up
      termui::terminal.MoveCursor(row, window_start_col + 4);
      termui::Text(key).width(key_w).height(1).align(termui::Text::Alignment::Right).render();

      termui::terminal.MoveCursor(row++, window_start_col + 4 + key_w + 2);
      termui::Text(description, termui::Styles::faint).width(_width - 6 - key_w - 2).height(1).render();
    }

    row++;
  }

  // control strip
  termui::terminal.MoveCursor(window_start_row + _height - 2, window_start_col + 2);
  kan::tui::ControlStrip({
                             {"esc", "close"},
                         })
      .width(_width - 4)
      .render();

  termui::terminal.flush();
}

void HelpDialog::process_input() {
  auto inp = termui::terminal.read();

  if (inp == termui::key::ESC || inp == "?" || inp == "q") {
    state = termui::Interface::State::Exit;
  }
}

void HelpDialog::show() {

  state = State::Continue;

  do {
    display();
    process_input();

  } while (state == State::Continue);
}

} // namespace tui
} // namespace kan
