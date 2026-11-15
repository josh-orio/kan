#include <kan/core.hpp>

namespace kan {

Event::Event() : uuid(UUID::generate()) {}

Event::Event(termui::string _text, User _user, Timestamp _time) : text(_text), user(_user), time(_time), uuid(UUID::generate()) {}

Event::Event(YAML::Node yaml) { fromYaml(yaml); }

void Event::fromYaml(YAML::Node yaml) {
  text = yaml["text"].as<std::string>();
  user = User().fromYaml(yaml["user"]);
  time = Timestamp().fromYaml(yaml["time"]);
  uuid = UUID::from_string(yaml["uuid"].as<std::string>());
}

YAML::Node Event::toYaml() {
  YAML::Node yaml;
  yaml["text"] = std::string(text);
  yaml["user"] = user.toYaml();
  yaml["time"] = time.toYaml();
  yaml["uuid"] = uuid.to_string();

  return yaml;
}

History::History() {}

History::History(YAML::Node yaml) { fromYaml(yaml); }

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
