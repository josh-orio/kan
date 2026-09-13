#include <kan/tui.hpp>

namespace kan {
namespace tui {

NewJob::NewJob() : new_job() {}

void NewJob::display() {
  auto terminal_half_w = termui::terminal.halfWidth();
  auto terminal_half_h = termui::terminal.halfHeight();

  auto window_start_row = terminal_half_h - (_height / 2);
  auto window_start_col = terminal_half_w - (_width / 2);

  // box
  termui::terminal.MoveCursor(window_start_row, window_start_col);
  termui::Box(_width, _height, termui::Borders::rounded, termui::Styles::faint).render();

  // title header
  termui::terminal.MoveCursor(window_start_row + 1, window_start_col + 2);
  termui::Text("Title", termui::Styles::bold).width(_width - 4).height(1).render();

  // title content
  termui::terminal.MoveCursor(window_start_row + 3, window_start_col + 2);
  termui::Input(new_job.title, "What needs to be done?").width(_width - 4).height(1).render();

  if (_selected_field == Focus::Title) {
    termui::terminal.SaveCursorPosition();
  }

  // description header
  termui::terminal.MoveCursor(window_start_row + 5, window_start_col + 2);
  termui::Text("Description", termui::Styles::bold).width(_width - 4).height(3).render();
  // termui::terminal.CursorDown(2).CursorLeft(_width - 4);

  // description content
  termui::terminal.MoveCursor(window_start_row + 7, window_start_col + 2);
  termui::Input(new_job.description, "Give some more info...").width(_width - 4).height(3).render();
  if (_selected_field == Focus::Description) {
    termui::terminal.SaveCursorPosition();
  }

  // window ctrl strip
  termui::terminal.MoveCursor(window_start_row + 11, window_start_col + 2);

  if (_selected_field != Focus::Finish) {
    kan::tui::ControlStrip({
                               {"esc", "close"},
                               {"↑↓", "navigate"},
                           })
        .width(_width - 4 - 1 - 8)
        .render();
  }
  else {
    kan::tui::ControlStrip({
                               {"esc", "close"},
                               {"↑↓", "navigate"},
                               {"↵", "submit"},
                           })
        .width(_width - 4 - 1 - 8)
        .render();
  }

  // 'finish' button
  termui::terminal.MoveCursor(window_start_row + 11, window_start_col + _width - 10);
  termui::Button b = termui::Button("Finish").focus_style(termui::Style({termui::SGR::Bold, termui::SGR::ReverseVideo})).blur_style(termui::Style()).width(8);

  if (_selected_field == Focus::Finish) {
    b.focus();
  }
  else {
    b.blur();
  }

  b.render();

  if (_selected_field != Focus::Finish) {
    termui::terminal.RestoreCursorPosition();
    termui::terminal.ShowCursor();
  }
  else {
    termui::terminal.HideCursor();
  }

  termui::terminal.flush();
}

void NewJob::process_input() {
  auto inp = termui::terminal.read();

  if (inp == termui::key::ESC) {
    state = termui::Interface::State::Exit;
  }
  else if (inp == termui::key::U_ARROW) {
    if (_selected_field == Focus::Description) {
      _selected_field = Focus::Title;
    }
    else if (_selected_field == Focus::Finish) {
      _selected_field = Focus::Description;
    }
  }
  else if (inp == termui::key::D_ARROW) {
    if (_selected_field == Focus::Title) {
      _selected_field = Focus::Description;
    }
    else if (_selected_field == Focus::Description) {
      _selected_field = Focus::Finish;
    }
  }
  else if ((std::string{32} <= inp) && (inp <= std::string{126})) { // accept basically all chars
    if (_selected_field == Focus::Title) {
      new_job.title += inp;
    }
    else if (_selected_field == Focus::Description) {
      new_job.description += inp;
    }
  }
  else if (inp == termui::key::DEL) { // remove last char
    if (_selected_field == Focus::Title) {
      if (new_job.title.size() > 0) {
        new_job.title = std::string(new_job.title.begin(), new_job.title.end() - 1);
      }
    }
    else if (_selected_field == Focus::Description) {
      if (new_job.description.size() > 0) {
        new_job.description = std::string(new_job.description.begin(), new_job.description.end() - 1);
      }
    }
  }
  else if (inp == termui::key::ENTER) {
    if (_selected_field == Focus::Title) {
      new_job.title += "\n";
    }
    else if (_selected_field == Focus::Description) {
      new_job.description += "\n";
    }
    else if (_selected_field == Focus::Finish) {
      state = termui::Interface::State::Submit;
    }
  }
}

std::pair<bool, kan::Job> NewJob::show() {
  state = State::Continue;

  do {
    display();
    process_input();

  } while (state == State::Continue);

  termui::terminal.HideCursor(); // just in case

  if (state == termui::Interface::State::Submit) {
    return {true, new_job};
  }
  else {
    return {false, new_job};
  }
}

} // namespace tui
} // namespace kan
