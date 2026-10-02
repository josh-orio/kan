#ifndef CORE_HISTORY_HPP
#define CORE_HISTORY_HPP

#include <kan/core/uuid.hpp>
#include <kan/core/timestamp.hpp>
#include <kan/core/user.hpp>
// #include <nlohmann/json.hpp>
#include <string>
#include <termui/termui.hpp>

namespace kan {

struct Event {
  termui::string text; // std::string text;
  User           user;
  Timestamp      time;
UUID    uuid;

  Event();
  Event(termui::string text, User user, Timestamp time);
  // Event(nlohmann::json json);
  Event(YAML::Node yaml);

  void       fromYaml(YAML::Node yaml);
  YAML::Node toYaml();

  // void           fromJson(nlohmann::json e);
  // nlohmann::json toJson();
};

struct History {
  std::vector<Event> events;

  History();
  // History(nlohmann::json json);
  History(YAML::Node yaml);

  // void           fromJson(nlohmann::json h);
  // nlohmann::json toJson();

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
