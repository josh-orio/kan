#include <kan/tui.hpp>

namespace kan {
namespace tui {

DiffViewer::DiffViewer(std::vector<std::string> lines) : _lines(lines) {}

DiffViewer &DiffViewer::size(unsigned int w, unsigned int h) {
  _w = w;
  _h = h;
  return *this;
}

unsigned int DiffViewer::maxStart() { return _lines.size() > _h ? _lines.size() - _h : 0; }

void DiffViewer::ScrollUp(unsigned int count) { _start_line = _start_line > count ? _start_line - count : 0; }

void DiffViewer::ScrollDown(unsigned int count) { _start_line = std::min(maxStart(), _start_line + count); }

void DiffViewer::GotoTop() { _start_line = 0; }

void DiffViewer::GotoBottom() { _start_line = maxStart(); }

void DiffViewer::render() {
  auto [row, col] = termui::terminal.GetFutureCursorPosition();

  if (_lines.empty()) {
    termui::Text("No changes to show.").style(termui::Styles::faint).width(_w).height(1).render();
    return;
  }

  _start_line = std::min(_start_line, maxStart());

  for (unsigned int i = _start_line; i < std::min<size_t>(_lines.size(), _start_line + _h); i++) {
    auto &line  = _lines.at(i);
    auto  style = termui::Style();

    if (line.starts_with("+")) {
      style = termui::Style(termui::Colors::Green, termui::Color::Inherit());
    }
    else if (line.starts_with("-")) {
      style = termui::Style(termui::Colors::Red, termui::Color::Inherit());
    }
    else if (line.starts_with("@@")) {
      style = termui::Style(termui::Colors::Cyan, termui::Color::Inherit(), {termui::SGR::Faint});
    }
    else if (line.starts_with("\\") || line.starts_with("Binary files")) { // "\ No newline at end of file"
      style = termui::Styles::faint;
    }

    termui::terminal.MoveCursor(row + (i - _start_line), col);
    termui::Text(line, style).width(_w).height(1).render();
  }
}

} // namespace tui
} // namespace kan
