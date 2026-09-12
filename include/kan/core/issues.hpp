#ifndef CORE_ISSUES_HPP
#define CORE_ISSUES_HPP

#include <kan/core/timestamp.hpp>
#include <kan/core/user.hpp>
#include <nlohmann/json.hpp>
#include <string>

namespace kan {

struct Issue {
  std::string description;
  User        creator;
  Timestamp   created;
  std::string uuid;

  Issue();
  Issue(nlohmann::json json);

  void fromJson(nlohmann::json json);

  nlohmann::json toJson();
};

struct Issues {
private:
  std::vector<Issue> _issues;

public:
  Issues();
  Issues(nlohmann::json json);

  void           fromJson(nlohmann::json json);
  nlohmann::json toJson();

  size_t size();
  Issue  at(int i);

  void addIssue(Issue c);
  void deleteIssue(int idx);
};

} // namespace kan

#endif
