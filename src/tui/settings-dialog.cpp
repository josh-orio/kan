#include <kan/tui.hpp>

namespace kan {
namespace tui {

// clears the area behind a centred box and draws it, giving back its top left corner
static std::pair<unsigned int, unsigned int> drawBox(unsigned int width, unsigned int height) {
  auto row = termui::terminal.halfHeight() - (height / 2);
  auto col = termui::terminal.halfWidth() - (width / 2);

  for (unsigned int r = 0; r < height; r++) {
    termui::terminal.MoveCursor(row + r, col);
    termui::Text("").width(width).height(1).render();
  }

  termui::terminal.MoveCursor(row, col);
  termui::Box(width, height, termui::Borders::rounded, termui::Styles::faint).render();

  return {row, col};
}

// ---- settings ----

bool SettingsDialog::onColumn() { return _cursor < board.columns.size(); }

void SettingsDialog::display() {
  auto &columns = board.columns;

  _cursor = std::min<unsigned int>(_cursor, columns.size()); // columns.size() is the option row
  _height = columns.size() + 11;

  auto [top, left] = drawBox(_width, _height);

  termui::terminal.MoveCursor(top + 1, left + 2);
  termui::Text("KAN I Change Some Settings?", termui::Styles::bold).width(_width - 4).height(1).render();

  // columns, each title drawn in its header style as a preview
  auto row = top + 3;

  termui::terminal.MoveCursor(row++, left + 2);
  termui::Text("Columns", termui::Styles::bold).width(_width - 4).height(1).render();

  for (unsigned int i = 0; i < columns.size(); i++, row++) {
    if (i == _cursor) {
      termui::terminal.MoveCursor(row, left + 2);
      termui::terminal.write("·");
    }

    termui::terminal.MoveCursor(row, left + 4);
    termui::PaddedText(columns[i].title, columns[i].header, termui::PaddingStyle::label).width(28).height(1).render();

    auto count = columns[i].Jobs().size();
    auto note  = std::format("{} {}", count, count == 1 ? "job" : "jobs");
    if (board.finish_on_last_column && i + 1 == columns.size()) {
      note += " · finishes jobs moved here";
    }

    termui::terminal.MoveCursor(row, left + 4 + 28 + 2);
    termui::Text(note, termui::Styles::faint).width(_width - 4 - 28 - 4).height(1).render();
  }

  // options
  row++;

  termui::terminal.MoveCursor(row++, left + 2);
  termui::Text("Options", termui::Styles::bold).width(_width - 4).height(1).render();

  if (_cursor == columns.size()) {
    termui::terminal.MoveCursor(row, left + 2);
    termui::terminal.write("·");
  }

  auto last = columns.empty() ? std::string("the last column") : columns.back().title;

  termui::terminal.MoveCursor(row, left + 4);
  termui::Text(board.finish_on_last_column ? "[x]" : "[ ]", termui::Styles::bold).width(3).height(1).render();
  termui::Text(std::format(" finish jobs moved into {}", last)).width(_width - 4 - 4 - 3).height(1).render();

  termui::terminal.MoveCursor(top + _height - 2, left + 2);

  auto strip = onColumn() ? ControlStrip({{"esc", "close"}, {"↵", "style"}, {"r", "rename"}, {"n", "new"}, {"< >", "move"}, {"D", "delete"}})
                          : ControlStrip({{"esc", "close"}, {"↵", "toggle"}, {"n", "new column"}});
  strip.width(_width - 4).render();

  termui::terminal.flush();
}

void SettingsDialog::process_input() {
  auto inp = termui::terminal.read();

  if (inp == termui::key::ESC || inp == "q") {
    state = termui::Interface::State::Exit;
  }
  else if (inp == termui::key::U_ARROW) {
    _cursor > 0 ? _cursor-- : _cursor;
  }
  else if (inp == termui::key::D_ARROW) {
    _cursor < board.columns.size() ? _cursor++ : _cursor;
  }
  else if (inp == "f" || (!onColumn() && (inp == termui::key::ENTER || inp == " "))) {
    board.finish_on_last_column = !board.finish_on_last_column;
    board.saveBoard();
  }
  else if (inp == "n") {
    addColumn();
  }
  else if (!onColumn()) {
    return;
  }
  else if (inp == termui::key::ENTER || inp == "s") {
    ColumnStyleDialog(_cursor).show();
  }
  else if (inp == "r") {
    renameColumn();
  }
  else if (inp == "D") {
    removeColumn();
  }
  else if (inp == "<" || inp == termui::key::SHIFT_U_ARROW) {
    moveColumn(-1);
  }
  else if (inp == ">" || inp == termui::key::SHIFT_D_ARROW) {
    moveColumn(1);
  }
}

// asks until the name is usable or the user gives up
static std::optional<std::string> askColumnName(const std::string &title, const std::string &current, int skip) {
  TextPrompt prompt(title, current, "Column name");

  while (auto name = prompt.show()) {
    name->erase(0, name->find_first_not_of(" "));
    name->erase(name->find_last_not_of(" ") + 1);

    if (auto problem = board.columnNameProblem(*name, skip); !problem.empty()) {
      prompt = TextPrompt(title, *name, "Column name").error(problem);
      continue;
    }

    return name;
  }

  return std::nullopt;
}

void SettingsDialog::addColumn() {
  auto at = onColumn() ? _cursor + 1 : board.columns.size(); // after the selected column

  if (auto name = askColumnName("New column", "", -1)) {
    board.addColumn(*name, at);
    _cursor = at;
  }
}

void SettingsDialog::renameColumn() {
  auto old = board.columns[_cursor].title;

  if (auto name = askColumnName(std::format("Rename {}", old), old, _cursor); name && *name != old) {
    board.renameColumn(_cursor, *name);
  }
}

void SettingsDialog::removeColumn() {
  auto &columns = board.columns;
  auto  title   = columns[_cursor].title;

  if (columns.size() == 1) {
    DialogBox::notice("Delete column", "A board needs at least one column.");
    return;
  }

  // its jobs need somewhere to go
  std::vector<std::string> others;
  std::vector<size_t>      other_idx;
  for (size_t i = 0; i < columns.size(); i++) {
    if (i != _cursor) {
      others.push_back(columns[i].title);
      other_idx.push_back(i);
    }
  }

  size_t to    = other_idx.front();
  auto   count = columns[_cursor].Jobs().size();

  if (count > 0) {
    auto [menu_state, choice] = MiniMenu(std::format("Move {} {} to", count, count == 1 ? "job" : "jobs"), others).show();

    if (menu_state != termui::Interface::State::Submit) {
      return;
    }

    to = other_idx.at(choice);
  }

  auto question = count > 0 ? std::format("Delete {} and move its {} {} to {}?", title, count, count == 1 ? "job" : "jobs", columns[to].title)
                            : std::format("Delete the column {}?", title);

  if (DialogBox("Delete column", question, "delete", "cancel").show()) {
    board.removeColumn(_cursor, to);
  }
}

void SettingsDialog::moveColumn(int by) {
  int to = static_cast<int>(_cursor) + by;

  if (to < 0 || to >= static_cast<int>(board.columns.size())) {
    return;
  }

  board.moveColumn(_cursor, to);
  _cursor = to;
}

void SettingsDialog::show() {
  state = State::Continue;

  do {
    display();
    process_input();

  } while (state == State::Continue);
}

// ---- column style ----

ColumnStyleDialog::ColumnStyleDialog(size_t column) : _column(column) {}

termui::Style ColumnStyleDialog::style() { return board.columns.at(_column).header; }

void ColumnStyleDialog::setStyle(termui::Style style) { board.setColumnHeader(_column, style); }

void ColumnStyleDialog::stepColor(int by) {
  auto  s     = style();
  auto &color = _cursor == 0 ? s.foregroundColor : s.backgroundColor;

  auto &colors = styling::palette();
  auto  it     = std::find(colors.begin(), colors.end(), color);
  int   idx    = it == colors.end() ? 0 : static_cast<int>(it - colors.begin()) + by; // a typed in colour starts over from default

  color = colors.at((idx + colors.size()) % colors.size());
  setStyle(s);
}

void ColumnStyleDialog::typeColor() {
  auto s     = style();
  auto title = _cursor == 0 ? std::string("Foreground") : std::string("Background");

  auto &color = _cursor == 0 ? s.foregroundColor : s.backgroundColor;

  TextPrompt prompt(title, styling::colorName(color), "default");
  prompt.hint("default, a name like red or bright-blue, 0-255, or #rrggbb");

  while (auto text = prompt.show()) {
    if (auto parsed = styling::parseColor(*text)) {
      color = *parsed;
      setStyle(s);
      return;
    }

    prompt = TextPrompt(title, *text, "default").error(std::format("{} isn't a colour - try red, 208 or #ff8800", *text));
  }
}

void ColumnStyleDialog::display() {
  auto  s     = style();
  auto &attrs = styling::attributes();

  auto [top, left] = drawBox(_width, _height);

  termui::terminal.MoveCursor(top + 1, left + 2);
  termui::Text("Column style", termui::Styles::bold).render();
  termui::Text(" " + board.columns.at(_column).title, termui::Styles::faint).width(_width - 4 - 12).height(1).render();

  // preview, the width of a column on the board-ish
  termui::terminal.MoveCursor(top + 3, left + 2);
  termui::PaddedText(board.columns.at(_column).title, s, termui::PaddingStyle::label).width(_width - 4).height(1).render();

  auto row = top + 5;

  auto indicator = [&](unsigned int i, unsigned int r) {
    if (i == _cursor) {
      termui::terminal.MoveCursor(r, left + 2);
      termui::terminal.write("·");
    }
  };

  for (unsigned int i = 0; i < 2; i++, row++) {
    indicator(i, row);

    termui::terminal.MoveCursor(row, left + 4);
    termui::Text(i == 0 ? "Foreground" : "Background").width(14).height(1).render();
    termui::Text(styling::colorName(i == 0 ? s.foregroundColor : s.backgroundColor), termui::Styles::bold).width(_width - 4 - 4 - 14 - 6).height(1).render();

    if (i == _cursor) {
      termui::terminal.MoveCursor(row, left + _width - 6);
      termui::Text("← →", termui::Styles::faint).width(3).height(1).render();
    }
  }

  row++;

  for (unsigned int i = 0; i < attrs.size(); i++, row++) {
    indicator(i + 2, row);

    bool on = std::find(s.attributes.begin(), s.attributes.end(), attrs[i].first) != s.attributes.end();

    termui::terminal.MoveCursor(row, left + 4);
    termui::Text(on ? "[x]" : "[ ]", termui::Styles::bold).width(3).height(1).render();
    termui::Text(" " + attrs[i].second).width(_width - 4 - 4 - 3).height(1).render();
  }

  termui::terminal.MoveCursor(top + _height - 2, left + 2);

  auto strip = _cursor < 2 ? ControlStrip({{"esc", "done"}, {"←→", "colour"}, {"↵", "type a colour"}, {"d", "default style"}})
                           : ControlStrip({{"esc", "done"}, {"↵", "toggle"}, {"d", "default style"}});
  strip.width(_width - 4).render();

  termui::terminal.flush();
}

void ColumnStyleDialog::process_input() {
  auto inp   = termui::terminal.read();
  auto count = 2 + styling::attributes().size();

  if (inp == termui::key::ESC || inp == "q") {
    state = termui::Interface::State::Exit;
  }
  else if (inp == termui::key::U_ARROW) {
    _cursor > 0 ? _cursor-- : _cursor;
  }
  else if (inp == termui::key::D_ARROW) {
    _cursor + 1 < count ? _cursor++ : _cursor;
  }
  else if (inp == "d") {
    setStyle(kan::Column::defaultHeader());
  }
  else if (_cursor < 2) {
    if (inp == termui::key::L_ARROW) {
      stepColor(-1);
    }
    else if (inp == termui::key::R_ARROW) {
      stepColor(1);
    }
    else if (inp == termui::key::ENTER) {
      typeColor();
    }
  }
  else if (inp == termui::key::ENTER || inp == " ") {
    auto s    = style();
    auto attr = styling::attributes().at(_cursor - 2).first;

    if (auto it = std::find(s.attributes.begin(), s.attributes.end(), attr); it != s.attributes.end()) {
      s.attributes.erase(it);
    }
    else { // kept in the order the list shows them
      s.attributes.push_back(attr);
      std::sort(s.attributes.begin(), s.attributes.end());
    }

    setStyle(s);
  }
}

void ColumnStyleDialog::show() {
  state = State::Continue;

  do {
    display();
    process_input();

  } while (state == State::Continue);
}

} // namespace tui
} // namespace kan
