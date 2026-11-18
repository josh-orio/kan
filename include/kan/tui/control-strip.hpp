#ifndef TUI_CONTROLSTRIP_HPP
#define TUI_CONTROLSTRIP_HPP

#include <initializer_list>
#include <string>
#include <vector>

namespace kan {
namespace tui {

class ControlStrip {
private:
  std::vector<std::pair<std::string, std::string>> _control_map;
  uint                                             _width;
  std::string                                      _seperator = "·";

public:
  ControlStrip(std::initializer_list<std::pair<std::string, std::string>> control_map);

  ControlStrip &width(uint n);
  ControlStrip &seperator(std::string s);

  void render();
};

} // namespace tui
} // namespace kan

#endif