#include <kan/tui.hpp>

namespace kan {
namespace tui {

TextPrompt::TextPrompt(const std::string &title, const std::string &text, const std::string &placeholder)
    : _title(title), _text(text), _placeholder(placeholder) {}

TextPrompt &TextPrompt::hint(const std::string &hint) {
  _hint = hint;
  return *this;
}

TextPrompt &TextPrompt::error(const std::string &error) {
  _error = error;
  return *this;
}

std::optional<std::string> TextPrompt::show() {
  state = State::Continue;

  do {
    display();
    process_input();

  } while (state == State::Continue);

  termui::terminal.HideCursor();

  if (state == State::Submit) {
    return _text;
  }
  return std::nullopt;
}

void TextPrompt::display() {
  auto window_start_row = termui::terminal.halfHeight() - (_height / 2);
  auto window_start_col = termui::terminal.halfWidth() - (_width / 2);

  // clear the area behind the box
  for (unsigned int r = 0; r < _height; r++) {
    termui::terminal.MoveCursor(window_start_row + r, window_start_col);
    termui::Text("").width(_width).height(1).render();
  }

  termui::terminal.MoveCursor(window_start_row, window_start_col);
  termui::Box(_width, _height, termui::Borders::rounded, termui::Styles::faint).render();

  termui::terminal.MoveCursor(window_start_row + 1, window_start_col + 2);
  termui::Text(_title, termui::Styles::bold).width(_width - 4).height(1).render();

  // only the end of a long answer fits
  auto input_w = _width - 4;
  auto shown   = _text.size() >= input_w ? _text.substr(_text.size() - input_w + 1) : _text;

  termui::terminal.MoveCursor(window_start_row + 3, window_start_col + 2);
  termui::Text(_text.empty() ? _placeholder : shown, _text.empty() ? termui::Styles::faint : termui::Style()).width(input_w).height(1).render();

  termui::terminal.MoveCursor(window_start_row + 5, window_start_col + 2);
  termui::Text(_error.empty() ? _hint : _error, _error.empty() ? termui::Styles::faint : termui::Styles::bold).width(_width - 4).height(1).render();

  termui::terminal.MoveCursor(window_start_row + _height - 2, window_start_col + 2);
  kan::tui::ControlStrip({{"↵", "save"}, {"esc", "cancel"}}).width(_width - 4).render();

  termui::terminal.MoveCursor(window_start_row + 3, window_start_col + 2 + termui::visible_length(shown));
  termui::terminal.ShowCursor();

  termui::terminal.flush();
}

void TextPrompt::process_input() {
  auto inp = termui::terminal.read();

  if (inp == termui::key::ESC) {
    state = State::Exit;
  }
  else if (inp == termui::key::ENTER) {
    state = State::Submit;
  }
  else if (inp == termui::key::DEL) {
    if (!_text.empty()) {
      _text.pop_back();
    }
  }
  else if (inp.size() == 1 && std::isprint(static_cast<unsigned char>(inp[0]))) {
    _text += inp;
    _error.clear();
  }
}

} // namespace tui
} // namespace kan
