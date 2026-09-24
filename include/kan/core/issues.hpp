#ifndef CORE_ISSUES_HPP
#define CORE_ISSUES_HPP

#include <kan/core/timestamp.hpp>
#include <kan/core/user.hpp>
// #include <nlohmann/json.hpp>
#include <string>

namespace kan {

struct Issue {
  std::string description;
  User        creator, modifier, assignee;
  Timestamp   created, modified;
  std::string uuid;

  enum class State { Open, Resolved };
  State state;

  Issue();
  // Issue(nlohmann::json json);
  Issue(YAML::Node yaml);

  // void fromJson(nlohmann::json json);
  // nlohmann::json toJson();

  void       fromYaml(YAML::Node yaml);
  YAML::Node toYaml();
};

struct Issues {
private:
  std::vector<Issue> _issues;

public:
  Issues();
  // Issues(nlohmann::json json);

  // void           fromJson(nlohmann::json json);
  // nlohmann::json toJson();

  void       fromYaml(YAML::Node yaml);
  YAML::Node toYaml();

  size_t size();
  Issue  at(int i);

  void addIssue(Issue i);
  void deleteIssue(int idx);
  void deleteIssue(Issue i);
};

} // namespace kan

#endif
