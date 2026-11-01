#include <kan/tui.hpp>

namespace kan {
namespace tui {

std::vector<std::variant<kan::Comment, kan::Commit>> idk(Job job) {
  std::vector<std::variant<kan::Comment, kan::Commit>> events;

  kan::Comment c;
  c.text    = job.description;
  c.creator = job.creator;
  c.created = job.created;

  // Comment {job.description,job.creator,job.created};
  events.push_back(c);

  for (const auto &c : job.comments) {
    events.push_back(c);
  }

  for (const auto &c: job.commits) {
    events.push_back(c);
  }

  // sort records (newest first)
  using Event = std::variant<kan::Comment, kan::Commit>;

  auto timeOf = [](const Event &e) {
    return std::visit([](const auto &x) { return x.created.time; }, e); // anything that will be visted needs to have a .created member
  };

  std::sort(events.begin(), events.end(), [&](const Event &a, const Event &b) { return timeOf(a) < timeOf(b); });

  return events;
}

std::string priorityStr(Job::Priority p) {
  switch (p) {
  case Job::Priority::Low:
    return "Low";
    break;
  case Job::Priority::Normal:
    return "Normal";
    break;
  case Job::Priority::High:
    return "High";
    break;
  }
}

std::string priorityIcon(Job::Priority p) {
  switch (p) {
  case Job::Priority::Low:
    return "∨";
    break;
  case Job::Priority::Normal:
    return "-";
    break;
  case Job::Priority::High:
    return "∧";
    break;
  }
}

JobDetails::JobDetails(kan::Job job) : _job(job), _events(idk(job)) {}

JobDetails &JobDetails::size(uint w, uint h) {
  _width  = w;
  _height = h;
  return *this;
}

JobDetails &JobDetails::CursorUp() {
  if (_focus == Focus::Assignee) {
    _focus = Focus::Status;
  }
  if (_focus == Focus::EventList) {
    if (_cursor == 0) {
      _focus = Focus::Assignee;
    }
    else {

      _cursor > 0 ? _cursor-- : _cursor;
      _cursor < _start_line ? _start_line = _cursor : _start_line;
    }
  }
  // else if (_selected_field == Focus::Archive) {
  //   _selected_field = Focus::Description;
  // }
  // else if (_selected_field == Focus::Delete) {
  //   _selected_field = Focus::Description;
  // }

  return *this;
}

JobDetails &JobDetails::CursorDown() {
  if (_focus == Focus::Status) {
    _focus = Focus::Assignee;
  }
  else if (_focus == Focus::Priority) {
    _focus = Focus::Assignee;
  }
  else if (_focus == Focus::Assignee) {
    _focus = Focus::EventList;
  }
  else if (_focus == Focus::EventList) {
    internal_update();

    _cursor + 1 < _events.size() ? _cursor++ : _cursor;
    _cursor >= _start_line + _visible_events ? _start_line = _cursor - _visible_events + 1 : _start_line;
  }
  // else if (_selected_field == Focus::Description) {
  //   _selected_field = Focus::Archive;
  // }
  // else if (_selected_field == Focus::Archive) {
  //   _selected_field = Focus::Delete;
  // }

  return *this;
}

JobDetails &JobDetails::CursorLeft() {
  if (_focus == Focus::Priority) {
    _focus = Focus::Status;
  }
  // else if (_selected_field == Focus::Delete) {
  //   _selected_field = Focus::Archive;
  // }

  return *this;
}

JobDetails &JobDetails::CursorRight() {
  if (_focus == Focus::Status) {
    _focus = Focus::Priority;
  }
  // else if (_selected_field == Focus::Archive) {
  //   _selected_field = Focus::Delete;
  // }

  return *this;
}

JobDetails &JobDetails::GotoTop() {
  if (_focus == Focus::EventList) {
    _cursor = 0;
  }

  return *this;
}

JobDetails &JobDetails::GotoBottom() {
  _focus  = Focus::EventList;
  _cursor = _events.size() - 1;

  return *this;
}

void JobDetails::Reload(Job job) {
  _job    = job;
  _events = idk(job);
}

JobDetails &JobDetails::editMode(bool x) {
  _edit_mode = x;
  return *this;
}

JobDetails::Focus JobDetails::Focussed() { return _focus; }

template <class... Ts> struct overloaded : Ts... {
  using Ts::operator()...;
};
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>; // needed pre-C++20

void JobDetails::render() {
  internal_update();

  auto [row, col] = termui::terminal.GetFutureCursorPosition();

  // status indicator
  if (_focus == Focus::Status) {
    termui::terminal.write("·");
  }

  // status
  termui::terminal.MoveCursor(row, col + 2);
  // termui::Text(_job.status, ::tui::stl(::tui::clrs::Red, ::tui::clr::Inherit())).width(_width - 2).height(1).render();
  termui::Text(_job.status).width(_width - 2).height(1).render();

  // priority (symbol + text)
  termui::terminal.MoveCursor(row, col + _width - 10);
  termui::terminal.write(priorityIcon(_job.priority));

  termui::terminal.MoveCursor(row, col + _width - 8);
  // termui::Text(priorityStr(_job.priority)).style(termui::Style(termui::Color::Inherit(), termui::Colors::Red, {termui::SGR::Bold})).width(6).render();
  termui::Text(priorityStr(_job.priority)).width(6).render();

  // priority indicator
  if (_focus == Focus::Priority) {
    termui::terminal.MoveCursor(row, col + _width - 1);
    termui::terminal.write("·");
  }

  // assignee indicator
  if (_focus == Focus::Assignee) {
    termui::terminal.MoveCursor(row + 2, col);
    termui::terminal.write("·");
  }

  // assignee
  termui::terminal.MoveCursor(row + 2, col + 2);
  termui::Text(_job.assignee.Name(), ::tui::stls::bold).width(_width - 2).height(1).render();

  // created
  termui::terminal.MoveCursor(row + _height - 4, col);
  termui::Text("Created: ").style(::tui::stls::bold).width(9).render();
  termui::Text(_job.creator.toString()).width((_width / 2) - 6 - 9).height(1).render();

  termui::terminal.MoveCursor(row + _height - 3, col + 9);
  termui::Text(_job.created.toString()).width((_width / 2) - 6 - 9).height(1).render();

  // updated
  termui::terminal.MoveCursor(row + _height - 4, col + (_width / 2));
  termui::Text("Updated: ").style(::tui::stls::bold).width(9).render();
  termui::Text(_job.modifier.toString()).width((_width / 2) - 6 - 9).height(1).render();

  termui::terminal.MoveCursor(row + _height - 3, col + (_width / 2) + 9);
  termui::Text(_job.updated.toString()).width((_width / 2) - 6 - 9).height(1).render();

  // help bar
  termui::terminal.MoveCursor(row + _height - 1, col);
  kan::tui::ControlStrip({
                             {"esc", "close"},
                             {"↑↓", "navigate"},
                             {"t", "top"},
                             {"b", "bottom"},
                             {"↵", "select"},
                             {"?", "help"},

                         })
      .width(_width - 1 - 9 - 1 - 8)
      .render();

  auto init_cursor_c = col + 2, init_cursor_r = row + 4;

  for (int i = _start_line; i < std::min(_events.size(), static_cast<size_t>(_start_line + _visible_events)); i++) {
    auto e = _events.at(i);

    unsigned int item_width, item_height;

    // clang-format off
    std::visit(overloaded{
      [&](const kan::Comment &c) {
        auto comment = tui::Comment(c).width(_width - 4);
        
        std::tie(item_width, item_height) = comment.size();

        termui::terminal.MoveCursor(init_cursor_r, init_cursor_c);

        if (_focus == Focus::EventList && i == _cursor) {
          comment.style(board.styleMap.at("highlight"));
        }

        comment.render();
      },
      [&](const Commit &c) {
        auto commit = tui::Commit(c).width(_width-4);

        std::tie(item_width, item_height) = commit.size();

        termui::terminal.MoveCursor(init_cursor_r, init_cursor_c);

        if (_focus == Focus::EventList && i == _cursor) {
          commit.style(board.styleMap.at("highlight"));
        }

        commit.render();
      }
    }, e);

    if (_focus == Focus::EventList && i == _cursor) {
      termui::terminal.MoveCursor(init_cursor_r, col);
      termui::Line(termui::Line::Direction::Vertical, item_height).render();
    }

    init_cursor_r += item_height + 1;
  }
  // clang-format on

  // // desc indicator
  // if (_selected_field == Focus::Description) {
  //   termui::terminal.MoveCursor(row + 2, col);
  //   // termui::Line(termui::Line::Direction::Vertical, _height - 7).render();
  //   termui::Line(termui::Line::Direction::Vertical, height).render();
  // }

  // // job description
  // termui::terminal.MoveCursor(row + 2, col + 2);
  // desc.render();
  // // termui::Input(_job.description).render();
  // // .height(_height - 7).valueStyle(board.styleMap.at(_edit_mode ? "selected_darkmode" : "")).render();
}

void JobDetails::internal_update() {
  std::vector<int> item_heights;

  for (const auto &e : _events) {

    // clang-format off
    std::visit(overloaded{
      [&](const kan::Comment &c) {
        auto comment = tui::Comment(c).width(_width -4);

        auto [width,height]= comment.size();

        item_heights.push_back(height);
      },
      [&](const Commit &c) {
        auto commit = tui::Commit(c).width(_width-4);

        auto [width,height]= commit.size();

        item_heights.push_back(height);
      }
    },e);
  }
  // clang-format on

  auto line = _start_line;

  _visible_events = 0;
  auto space_used = 0;
  while (true) {
    if (line >= _events.size()) {
      break;
    }

    space_used += item_heights.at(line) + 1;

    if (space_used > _height - 8) {
      break;
    }

    _visible_events++;
    line++;
  }

  auto tmp = 0;

  // keep cursor in visible bounds
  if (_start_line > _cursor) {
    _start_line = _cursor;
  }
  if (_cursor > _start_line + _visible_events - 1) {
    _start_line = _cursor - _visible_events + 1;
  }
}

} // namespace tui
} // namespace kan
