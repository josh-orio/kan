#include <kan/core.hpp>

namespace kan {

// ---- Commit ----
Commit::Commit() : uuid(UUID::generate()) {}

Commit::Commit(YAML::Node yaml) { fromYaml(yaml); }

void Commit::fromYaml(YAML::Node yaml) {
  message = yaml["message"].as<std::string>();
  creator = User().fromYaml(yaml["creator"]);
  created = Timestamp().fromYaml(yaml["created"]);
  uuid    = UUID::from_string(yaml["uuid"].as<std::string>());
}

YAML::Node Commit::toYaml() {
  YAML::Node yaml;
  yaml["message"] = message;
  yaml["creator"] = creator.toYaml();
  yaml["created"] = created.toYaml();
  yaml["uuid"]    = uuid.to_string();

  return yaml;
}

// ---- Commits ----
Commits::Commits() : _commits() {}

Commits::Commits(YAML::Node yaml) { fromYaml(yaml); }

void Commits::fromYaml(YAML::Node yaml) {
  _commits.clear();

  for (const auto &c : yaml) {
    _commits.push_back(Commit(c));
  }
}

YAML::Node Commits::toYaml() {
  YAML::Node yaml(YAML::NodeType::Sequence);

  for (auto &c : _commits) {
    yaml.push_back(c.toYaml());
  }

  return yaml;
}

std::vector<Commit>::iterator Commits::begin() { return _commits.begin(); }

std::vector<Commit>::iterator Commits::end() { return _commits.end(); }

size_t Commits::size() { return _commits.size(); }

Commit Commits::at(int i) { return _commits.at(i); }

void Commits::addCommit(Commit c) { _commits.push_back(c); }

void Commits::deleteCommit(int idx) { _commits.erase(_commits.begin() + idx); }

void Commits::deleteCommit(Commit c) {
  _commits.erase(std::remove_if(_commits.begin(), _commits.end(), [&](const kan::Commit &commit) { return commit.uuid == c.uuid; }), _commits.end());
}

} // namespace kan
