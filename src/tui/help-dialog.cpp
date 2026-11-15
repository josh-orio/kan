#include <kan/tui.hpp>

namespace kan {
namespace tui {

// sections of the help dialog - an empty key means the description is a line of prose
struct HelpSection {
  std::string                                      title;
  std::vector<std::pair<std::string, std::string>> rows;
};

static const std::vector<HelpSection> sections = {
    {"Board",
     {
         {"←↑↓→", "move between jobs"},
         {"o  ↵", "open the selected job"},
         {"n", "new job in this column"},
         {"s", "settings"},
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
         {"", "Commits tab."},
     }},
};

void HelpDialog::display() {
  auto key_w = 6u;

  // fit the box to the content: title + blank, then each section's header, rows and a blank line, then the control strip
  _width  = 56;
  _height = 3 + 2;
  for (const auto &section : sections) {
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

  for (const auto &section : sections) {
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
