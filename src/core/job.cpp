#include <kan/core.hpp>

namespace kan {

Job::Job() : title(), description(), status(), priority(Priority::Normal), created(), updated(), creator(), uuid(UUID::generate().to_string()) {
  created = Timestamp().Now();
  updated = created;

  creator = User().getFromSystem();
}

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
  id          = yaml["id"].as<int>(0);

  comments = Comments(yaml["comments"]);
  commits  = Commits(yaml["commits"]);
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
  job["id"]          = id;

  job["comments"] = comments.toYaml();
  job["commits"]  = commits.toYaml();
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
