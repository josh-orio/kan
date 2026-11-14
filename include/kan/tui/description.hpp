#ifndef TUI_DESCRIPTION_HPP
#define TUI_DESCRIPTION_HPP

#include <kan/core.hpp>
#include <termui/termui.hpp>

namespace kan {
namespace tui {

// the job description, shown at the top of the job details event list
struct JobDescription {
  std::string text;
  User        creator;
  Timestamp   created;
};

class Description {
private:
  JobDescription     _description;
  std::optional<int> _width, _height;
  termui::Style      _style;

public:
  Description(JobDescription description);

  Description &width(std::optional<int> n);
  Description &height(std::optional<int> n);

  Description &style(termui::Style s);

  std::pair<unsigned int, unsigned int> size();

  void render();
};

} // namespace tui
} // namespace kan

#endif
