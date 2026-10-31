#ifndef CORE_JOB_HPP
#define CORE_JOB_HPP

#include <kan/core/comments.hpp>
#include <kan/core/commits.hpp>
#include <kan/core/history.hpp>
#include <kan/core/issues.hpp>
#include <kan/core/user.hpp>
#include <kan/core/uuid.hpp>
#include <string>

// #include <nlohmann/json.hpp>
#include <yaml-cpp/yaml.h>

namespace kan {

struct Job {
  std::string title, description, status, uuid;

  enum class Priority { Low, Normal, High };
  Priority priority;

  User      creator, modifier, assignee;
  Timestamp created, updated;

  Comments comments;
  Commits commits;
  History history;


  Job();
  // Job(nlohmann::json json);
  Job(YAML::Node yaml);

  // void           fromJson(nlohmann::json job);
  // nlohmann::json toJson();

  void       fromYaml(YAML::Node yaml);
  YAML::Node toYaml();

  void addEvent(Event e);
};

} // namespace kan

namespace YAML {

template <> struct convert<kan::Job::Priority> {
  static Node encode(const kan::Job::Priority &rhs) {
    switch (rhs) {
    case kan::Job::Priority::Low:
      return Node("Low");
    case kan::Job::Priority::Normal:
      return Node("Normal");
    case kan::Job::Priority::High:
      return Node("High");
      // ...add all enumerators
    }
    throw std::runtime_error("Unknown Priority value");
  }

  static bool decode(const Node &node, kan::Job::Priority &rhs) {
    if (!node.IsScalar())
      return false;
    const std::string &val = node.Scalar();
    if (val == "Low") {
      rhs = kan::Job::Priority::Low;
    }
    else if (val == "Normal") {
      rhs = kan::Job::Priority::Normal;
    }
    else if (val == "High") {
      rhs = kan::Job::Priority::High;
    }
    else {
      return false;
    }
    return true;
  }
};

} // namespace YAML

#endif
