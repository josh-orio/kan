#ifndef TUI_NEWJOB_HPP
#define TUI_NEWJOB_HPP

#include <kan/core.hpp>
#include <termui/termui.hpp>

namespace kan {
namespace tui {

class NewJob : termui::Interface {
  uint _width = 80, _height = 13;

  enum class Focus { Title, Description, Finish };
  Focus _selected_field = Focus::Title;

  kan::Job new_job;

  void display();
  void process_input();

public:
  NewJob();

  std::pair<bool, kan::Job> show();
};

} // namespace tui
} // namespace kan

#endif
