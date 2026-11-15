#include <kan/tui.hpp>

namespace kan {
namespace tui {

static termui::Style lineStyle(const std::string &line) {
  if (line.starts_with("+")) {
    return termui::Style(termui::Colors::Green, termui::Color::Inherit());
  }
  if (line.starts_with("-")) {
    return termui::Style(termui::Colors::Red, termui::Color::Inherit());
  }
  if (line.starts_with("@@")) {
    return termui::Style(termui::Colors::Cyan, termui::Color::Inherit(), {termui::SGR::Faint});
  }
  if (line.starts_with("\\") || line.starts_with("Binary files")) { // "\ No newline at end of file"
    return termui::Styles::faint;
  }

  return termui::Style();
}

DiffViewer::DiffViewer(std::vector<std::string> lines) : _lines(lines) {
  // pair each run of removed lines with the run of added lines after it, context lines go on both sides
  std::vector<std::string> removed, added;

  auto flush = [&]() {
    for (size_t i = 0; i < std::max(removed.size(), added.size()); i++) {
      Row r;
      if (i < removed.size()) {
        r.left = removed[i];
      }
      if (i < added.size()) {
        r.right = added[i];
      }
      _rows.push_back(r);
    }

    removed.clear();
    added.clear();
  };

  for (const auto &line : _lines) {
    if (line.starts_with("-")) {
      if (!added.empty()) { // a removal after additions starts a new change block
        flush();
      }
      removed.push_back(line);
    }
    else if (line.starts_with("+")) {
      added.push_back(line);
    }
    else {
      flush();

      if (!line.starts_with(" ")) { // hunk headers, "\ No newline...", binary notes etc span both sides
        _rows.push_back({std::nullopt, std::nullopt, line});
      }
      else {
        _rows.push_back({line, line, ""});
      }
    }
  }

  flush();
}

DiffViewer &DiffViewer::size(unsigned int w, unsigned int h) {
  _w = w;
  _h = h;
  return *this;
}

DiffViewer &DiffViewer::sideBySide(bool x) {
  _side_by_side = x;
  _start_line   = std::min(_start_line, maxStart());
  return *this;
}

bool DiffViewer::SideBySide() { return _side_by_side; }

unsigned int DiffViewer::rowCount() { return _side_by_side ? _rows.size() : _lines.size(); }

unsigned int DiffViewer::maxStart() { return rowCount() > _h ? rowCount() - _h : 0; }

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

  if (_side_by_side) {
    renderSideBySide(row, col);
  }
  else {
    renderUnified(row, col);
  }
}

void DiffViewer::renderUnified(unsigned int row, unsigned int col) {
  for (unsigned int i = _start_line; i < std::min<size_t>(_lines.size(), _start_line + _h); i++) {
    termui::terminal.MoveCursor(row + (i - _start_line), col);
    termui::Text(_lines.at(i), lineStyle(_lines.at(i))).width(_w).height(1).render();
  }
}

void DiffViewer::renderSideBySide(unsigned int row, unsigned int col) {
  // old | new, split down the middle with a faint divider
  auto side_w  = _w > 3 ? (_w - 3) / 2 : 0;
  auto right_c = col + side_w + 3;

  for (unsigned int i = _start_line; i < std::min<size_t>(_rows.size(), _start_line + _h); i++) {
    auto &r = _rows.at(i);
    auto  y = row + (i - _start_line);

    if (!r.header.empty()) {
      termui::terminal.MoveCursor(y, col);
      termui::Text(r.header, lineStyle(r.header)).width(_w).height(1).render();
      continue;
    }

    // context lines carry their leading space, changed lines their +/-; both sides drop that marker
    termui::terminal.MoveCursor(y, col);
    if (r.left) {
      termui::Text(r.left->substr(std::min<size_t>(1, r.left->size())), lineStyle(*r.left)).width(side_w).height(1).render();
    }

    termui::terminal.MoveCursor(y, col + side_w + 1);
    termui::Text("│", termui::Styles::faint).width(1).height(1).render();

    termui::terminal.MoveCursor(y, right_c);
    if (r.right) {
      termui::Text(r.right->substr(std::min<size_t>(1, r.right->size())), lineStyle(*r.right)).width(side_w).height(1).render();
    }
  }
}

} // namespace tui
} // namespace kan
