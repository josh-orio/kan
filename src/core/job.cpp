#include <kan/core.hpp>

namespace kan {

// NLOHMANN_JSON_SERIALIZE_ENUM(Job::Priority, {
//                                                 {Job::Priority::Low, "Low"},
//                                                 {Job::Priority::Normal, "Normal"},
//                                                 {Job::Priority::High, "High"},
//                                             })

Job::Job() : title(), description(), status(), priority(Priority::Normal), created(), updated(), creator(), uuid(UUID::generate().to_string()) {
  created = Timestamp().Now();
  updated = created;

  creator = User().getFromSystem();
}

// Job::Job(nlohmann::json json) { fromJson(json); }

// nlohmann::json Job::toJson() {
//   nlohmann::ordered_json job;
//   job["title"]       = title;
//   job["description"] = description;
//   job["status"]      = status;
//   job["created"]     = created.toJson();
//   job["updated"]     = updated.toJson();
//   job["creator"]     = creator.toJson();
//   job["uuid"]        = uuid;

//   job["comments"] = comments.toJson();
//   job["priority"] = priority;
//   job["history"]  = history.toJson();

//   return job;
// };

// void Job::fromJson(nlohmann::json json) {
//   // std::string tmp = json.dump();

//   title       = json["title"];
//   description = json["description"];
//   status      = json["status"];
//   created     = Timestamp(json["created"]);
//   updated     = Timestamp(json["updated"]);
//   uuid        = json["uuid"];

//   // comments = Comments(json.value("comments", nlohmann::json::object()));

//   priority = json.value("priority", Priority::Normal);

//   // history = History(json.value("history", nlohmann::json::object()));
// };

Job::Job(YAML::Node yaml) { fromYaml(yaml); }

void Job::fromYaml(YAML::Node yaml) {
  title       = yaml["title"].as<std::string>();
  description = yaml["description"].as<std::string>();
  status      = yaml["status"].as<std::string>();
  created     = Timestamp(yaml["created"]);
  updated     = Timestamp(yaml["updated"]);
  creator     = User().fromYaml(yaml["creator"]);
  modifier    = User().fromYaml(yaml["modifier"]);
  assignee     = User().fromYaml(yaml["assignee"]);
  uuid        = yaml["uuid"].as<std::string>();

  comments = Comments(yaml["comments"]);
  priority = yaml["priority"].as<Priority>();
  history  = History(yaml["history"]);
}

YAML::Node Job::toYaml() {
  YAML::Node job;
  job["title"]       = title;
  job["description"] = description;
  job["status"]      = status;
  job["created"]     = created.toYaml();
  job["updated"]     = updated.toYaml();
  job["creator"]     = creator.toYaml();
  job["modifier"]    = modifier.toYaml();
  job["assignee"]    = assignee.toYaml();
  job["uuid"]        = uuid;

  job["comments"] = comments.toYaml();
  job["priority"] = priority;
  job["history"]  = history.toYaml();

  return job;
}

void Job::addEvent(Event e) {
  history.addEvent(e);

  modifier = history.events.back().user;
  updated  = history.events.back().time;
}

} // namespace kan
