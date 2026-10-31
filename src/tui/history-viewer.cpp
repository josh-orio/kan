#include <kan/tui.hpp>

namespace kan {
namespace tui {

HistoryViewer::HistoryViewer(History history) : _history(history), _start_line(0), _cursor(0) {}

HistoryViewer &HistoryViewer::size(unsigned int w, unsigned int h) {
  _w = w;
  _h = h;
  return *this;
}

HistoryViewer &HistoryViewer::width(unsigned int w) {
  _w = w;
  return *this;
}

HistoryViewer &HistoryViewer::height(unsigned int h) {
  _h = h;
  return *this;
}

void HistoryViewer::Reload(History h) { _history = h; }

void HistoryViewer::CursorUp(unsigned int count) {
  internal_update();
  _cursor     = (_cursor > 0) ? std::max(0, int(_cursor - count)) : 0; // decrement but dont let (cursor < 0)
  _start_line = (_cursor < _start_line) ? _cursor : _start_line;       // don't let cursor go 'above' start line
}

void HistoryViewer::CursorDown(unsigned int count) {
  internal_update();
  _cursor     = (_cursor < _history.size() - count) ? std::min(_history.size() - 1, static_cast<size_t>(_cursor + count))
                                                    : _history.size() - 1;                            // increment but dont let (cursor > elements.size)
  _start_line = (_cursor >= _start_line + _visible_rows) ? _cursor - _visible_rows + 1 : _start_line; // don't let cursor go 'below' bottom of visible lines
}

unsigned int HistoryViewer::GetCursor() { return _cursor; }

void HistoryViewer::render() {
  internal_update();

  auto [row, col] = termui::terminal.GetFutureCursorPosition();

  if (_history.size() == 0) {
    termui::Text("No history to show.").style(termui::Styles::faint).align(termui::Text::Alignment::Center).width(_w).height(1).render();
  }
  else {
    auto row_style = board.styleMap.at("highlight");

    for (int i = _start_line; i < std::min(_history.size(), static_cast<size_t>(_start_line + _visible_rows)); i++) {
      auto start_row = row + (3 * (i - _start_line));

      // if (i + _start_line == _cursor) {
      if (i == _cursor) {
        termui::terminal.MoveCursor(start_row, col);
        termui::Line(termui::Line::Direction::Vertical, 2).render();
      }

      termui::terminal.MoveCursor(start_row, col + 2);
      termui::Text(_history.at(i).text).size(_w - 2, 1).style(i == _cursor ? row_style : termui::Style()).render();

      termui::terminal.MoveCursor(start_row + 1, col + 2);

      auto bold_style = i == _cursor ? row_style : termui::Style();
      bold_style.attributes = {termui::SGR::Bold};

      termui::Text(_history.at(i ).user.Name()).style(termui::Styles::bold).size(_w - 2 - 20, 1).style(bold_style).render();
      termui::Text(_history.at(i ).time.toString()).size(20, 1).align(termui::Text::Alignment::Right).style(i == _cursor ? row_style : termui::Style()).render();
    }
  }

  termui::terminal.MoveCursor(row + _h - 1, col);
  kan::tui::ControlStrip({
                             {"esc", "close"},
                             {"↑↓", "navigate"},
                             {"o/↵", "select"},
                             {"c", "new comment"},
                         })
      .width(_w)
      .render();
}

void HistoryViewer::internal_update() { // add args for recalcRows and sort to prevent waste
  _visible_rows = 0;
  while (true) {
    // calculates lines used by displaying another row of cells
    int space_used = (_cell_height * (_visible_rows + 1)) + (_line_spacing * _visible_rows);

    if (space_used > _h - 2) { // -2 for controlstrip space
      break;
    }
    _visible_rows++;
  }

  // sort records (newest first)
  std::sort(_history.events.begin(), _history.events.end(), [](const Event &a, const Event &b) { return a.time.time > b.time.time; });
}

} // namespace tui
} // namespace kan
