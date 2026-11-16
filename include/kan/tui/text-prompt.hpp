#ifndef TUI_TEXT_PROMPT_HPP
#define TUI_TEXT_PROMPT_HPP

#include <optional>
#include <termui/termui.hpp>

namespace kan {
namespace tui {

// asks for one line of text, looks like DialogBox. ↵ gives back the text, esc gives back nothing
class TextPrompt : public termui::Interface {
public:
  TextPrompt(const std::string &title, const std::string &text = "", const std::string &placeholder = "");

  TextPrompt &hint(const std::string &hint);   // faint line under the input
  TextPrompt &error(const std::string &error); // replaces the hint, eg why the last answer didn't work

  std::optional<std::string> show();

private:
  std::string _title, _text, _placeholder, _hint, _error;

  size_t _width = 60, _height = 9;

  void display();
  void process_input();
};

} // namespace tui
} // namespace kan

#endif
