#ifndef TUI_HELP_DIALOG_HPP
#define TUI_HELP_DIALOG_HPP

#include <kan/core.hpp>
#include <termui/termui.hpp>

namespace kan {
namespace tui {

// a group of keys in the help dialog - an empty key means the description is a line of prose
struct HelpSection {
  std::string                                      title;
  std::vector<std::pair<std::string, std::string>> rows;
};

extern const std::vector<HelpSection> boardHelp, jobWindowHelp;

class HelpDialog : termui::Interface {
private:
  uint _width = 80, _height = 40;

  std::vector<HelpSection> _sections;

  void display();
  void process_input();

public:
  HelpDialog(std::vector<HelpSection> sections = boardHelp);

  void show();
};

} // namespace tui
} // namespace kan

#endif
