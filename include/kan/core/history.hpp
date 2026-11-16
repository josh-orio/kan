#ifndef CORE_HISTORY_HPP
#define CORE_HISTORY_HPP

#include <kan/core/timestamp.hpp>
#include <kan/core/user.hpp>
#include <kan/core/uuid.hpp>
#include <string>
#include <termui/termui.hpp>

namespace kan {

struct Event {
  termui::string text;
  User           user;
  Timestamp      time;
  UUID           uuid;

  Event();
  Event(termui::string text, User user, Timestamp time);
  Event(YAML::Node yaml);

  void       fromYaml(YAML::Node yaml);
  YAML::Node toYaml();
};

struct History {
  std::vector<Event> events;

  History();
  History(YAML::Node yaml);

  void       fromYaml(YAML::Node yaml);
  YAML::Node toYaml();

  size_t size();
  Event  at(int idx);

  void addEvent(Event e);
  void deleteEvent(int idx);
  void deleteEvent(Event e);
};

} // namespace kan

#endif
