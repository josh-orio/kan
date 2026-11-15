#ifndef TUI_INPUT_HPP
#define TUI_INPUT_HPP

#include <string>

namespace kan {
namespace tui {

std::string readKey(unsigned int timeout_ms); // like terminal.read(), but gives up with "" when nothing is pressed in time

} // namespace tui
} // namespace kan

#endif
