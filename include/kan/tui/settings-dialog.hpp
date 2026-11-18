#ifndef TUI_SETTINGS_WINDOW_HPP
#define TUI_SETTINGS_WINDOW_HPP

#include <kan/core.hpp>
#include <termui/termui.hpp>

namespace kan {
namespace tui {

// the board's columns (add, rename, remove, reorder, style) and its options
class SettingsDialog : termui::Interface {
private:
  uint         _width = 72, _height = 12;
  unsigned int _cursor = 0; // the columns come first, then the options

  enum Option { Name, FinishOnLast, OptionCount }; // the option rows, in order, after the columns

  bool onColumn();
  bool onOption(Option o);

  void renameBoard();

  void addColumn();
  void renameColumn();
  void removeColumn();
  void moveColumn(int by);

  void display();
  void process_input();

public:
  SettingsDialog() = default;

  void show(); // columns can be added and removed, anything pointing into board.columns needs rebuilding after
};

// foreground, background and attributes for one column's header
class ColumnStyleDialog : termui::Interface {
private:
  size_t       _column;
  uint         _width = 60, _height = 17;
  unsigned int _cursor = 0; // foreground, background, then the attributes

  termui::Style style();
  void          setStyle(termui::Style style);

  void stepColor(int by);
  void typeColor();

  void display();
  void process_input();

public:
  ColumnStyleDialog(size_t column);

  void show();
};

} // namespace tui
} // namespace kan

#endif
