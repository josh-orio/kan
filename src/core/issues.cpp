#include <kan/core.hpp>

namespace kan {

// ---- Issue ----
Issue::Issue() : uuid(UUID::generate()) {}

Issue::Issue(YAML::Node yaml) { fromYaml(yaml); }

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
