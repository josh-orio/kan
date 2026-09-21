#include <kan/core.hpp>

namespace kan {

Event::Event() : uuid(UUIDv7::generate().to_string()) {}

Event::Event(termui::string _text, User _user, Timestamp _time) : text(_text), user(_user), time(_time), uuid(UUIDv7::generate().to_string()) {}

// Event::Event(nlohmann::json json) { fromJson(json); }

Event::Event(YAML::Node yaml) { fromYaml(yaml); }

// void Event::fromJson(nlohmann::json e) {
//   text = termui::string(e.at("text"));
//   user = User().fromJson(e.at("user"));
//   time = Timestamp().fromJson(e.at("time"));
//   uuid = termui::string(e.at("uuid"));
// }

// nlohmann::json Event::toJson() {
//   return {
//       {"text", text},
//       {"user", user.toJson()},
//       {"time", time.toJson()},
//       {"uuid", uuid},
//   };
// }

void Event::fromYaml(YAML::Node yaml) {
  text = yaml["text"].as<std::string>();
  user = User().fromYaml(yaml["user"]);
  time = Timestamp().fromYaml(yaml["time"]);
  uuid = yaml["uuid"].as<std::string>();
}

YAML::Node Event::toYaml() {
  YAML::Node yaml;
  yaml["text"] = std::string(text);
  yaml["user"] = user.toYaml();
  yaml["time"] = time.toYaml();
  yaml["uuid"] = uuid;

  return yaml;
}

History::History() {}

// History::History(nlohmann::json json) { fromJson(json); }

History::History(YAML::Node yaml) { fromYaml(yaml); }

// void History::fromJson(nlohmann::json h) {
//   _events.clear();

//   // if (_events.empty() || h.is_null()) {
//   if (h.is_null()) {
//     return;
//   }

//   for (auto &c : h) {
//     _events.push_back(Event(c));
//   }
// }

// nlohmann::json History::toJson() {
//   auto json = nlohmann::json::array();

//   for (auto &e : _events) {
//     json.push_back(e.toJson());
//   }

//   return json;
// }

void History::fromYaml(YAML::Node yaml) {
  events.clear();

  for (const auto &e : yaml) {
    events.push_back(Event(e));
  }
}

YAML::Node History::toYaml() {
  YAML::Node yaml(YAML::NodeType::Sequence);

  for (auto &e : events) {
    yaml.push_back(e.toYaml());
  }

  return yaml;
}

size_t History::size() { return events.size(); }

Event History::at(int idx) { return events.at(idx); }

void History::addEvent(Event e) { events.push_back(e); }

void History::deleteEvent(int idx) { events.erase(events.begin() + idx); }

void History::deleteEvent(Event e) {
  events.erase(std::remove_if(events.begin(), events.end(), [&](const kan::Event &event) { return event.uuid == e.uuid; }), events.end());
}

} // namespace kan
