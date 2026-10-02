#include <kan/core.hpp>

namespace kan {

// ---- Issue ----
Issue::Issue() : uuid(UUID::generate()){}

// Issue::Issue(nlohmann::json json) { fromJson(json); }

Issue::Issue(YAML::Node yaml) { fromYaml(yaml); }

// void Issue::fromJson(nlohmann::json issue) {
//   description = termui::string(issue.at("description"));
//   creator     = User().fromJson(issue.at("creator"));
//   created     = Timestamp().fromJson(issue.at("created"));
//   uuid        = issue.at("uuid");
// }

// nlohmann::json Issue::toJson() {
//   return {
//       {"description", description},
//       {"creator", creator.toJson()},
//       {"created", created.toJson()},
//       {"uuid", uuid},
//   };
// }

void Issue::fromYaml(YAML::Node yaml) {
  description = yaml["description"].as<std::string>();
  creator     = User().fromYaml(yaml["creator"]);
  created     = Timestamp().fromYaml(yaml["created"]);
  uuid        = UUID::from_string(yaml["uuid"].as<std::string>());
}

YAML::Node Issue::toYaml() {
  YAML::Node yaml;
  yaml["description"] = std::string(description);
  yaml["creator"]     = creator.toYaml();
  yaml["created"]     = created.toYaml();
  yaml["uuid"]        = uuid.to_string();

  return yaml;
}

// ---- Issues ----
Issues::Issues() {}

// Issues::Issues(nlohmann::json json) { fromJson(json); }

// void Issues::fromJson(nlohmann::json issues) {
//   _issues.clear();

//   if (issues.empty() || issues.is_null()) {
//     return;
//   }

//   for (auto &i : issues) {
//     _issues.push_back(Issue(i));
//   }
// }

// nlohmann::json Issues::toJson() {
//   auto json = nlohmann::json::array();

//   for (auto &i : _issues) {
//     json.push_back(i.toJson());
//   }

//   return json;
// }

void Issues::fromYaml(YAML::Node yaml) {
  _issues.clear();

  for (const auto &i : yaml) {
    _issues.push_back(Issue(i));
  }
}

YAML::Node Issues::toYaml() {
  YAML::Node yaml(YAML::NodeType::Sequence);

  for (auto &i : _issues) {
    yaml.push_back(i.toYaml());
  }

  return yaml;
}

size_t Issues::size() { return _issues.size(); }

Issue Issues::at(int i) { return _issues.at(i); }

void Issues::addIssue(Issue i) { _issues.push_back(i); }

void Issues::deleteIssue(int idx) { _issues.erase(_issues.begin() + idx); }

void Issues::deleteIssue(Issue i) {
  _issues.erase(std::remove_if(_issues.begin(), _issues.end(), [&](const kan::Issue &issue) { return issue.uuid == i.uuid; }), _issues.end());
}

} // namespace kan
