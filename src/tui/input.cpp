#include <kan/tui.hpp>

#include <termios.h>
#include <unistd.h>

namespace kan {
namespace tui {

std::string readKey(unsigned int timeout_ms) {
  termios old, timed;
  tcgetattr(STDIN_FILENO, &old);

  // VMIN 0 + VTIME makes read() return empty handed after VTIME tenths of a second
  timed             = old;
  timed.c_cc[VMIN]  = 0;
  timed.c_cc[VTIME] = std::clamp(timeout_ms / 100, 1u, 255u);
  tcsetattr(STDIN_FILENO, TCSANOW, &timed);

  auto key = termui::terminal.read();

  tcsetattr(STDIN_FILENO, TCSANOW, &old);

  return key;
}

} // namespace tui
} // namespace kan
