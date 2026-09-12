#ifndef CORE_JOB_HPP
#define CORE_JOB_HPP

#include <kan/core/comments.hpp>
#include <kan/core/issues.hpp>
#include <kan/core/user.hpp>
#include <kan/core/uuid.hpp>
#include <nlohmann/json.hpp>
#include <string>

namespace kan {

struct Job {
  std::string title, description, status, uuid;

  User   creator, modifier;
  time_t created, updated;

  Comments                 comments;
  Issues                   issues;
  std::vector<std::string> history;

  Job();
  Job(nlohmann::json json);

  void           fromJson(nlohmann::json job);
  nlohmann::json toJson();
};

} // namespace kan

#endif
