#ifndef CORE_COMMITS_HPP
#define CORE_COMMITS_HPP

#include <kan/core/uuid.hpp>
#include <kan/core/timestamp.hpp>
#include <kan/core/user.hpp>
// #include <nlohmann/json.hpp>
#include <string>
#include <termui/termui.hpp>

namespace kan {

struct Commit {
  std::string message;
  User           creator;
  Timestamp      created;
  UUID           uuid;

  Commit();
  Commit(YAML::Node yaml);

  void       fromYaml(YAML::Node yaml);
  YAML::Node toYaml();
};

struct Commits {
private:
  std::vector<Commit> _commits;

public:
  Commits();
  Commits(YAML::Node yaml);

  void       fromYaml(YAML::Node yaml);
  YAML::Node toYaml();

  size_t  size();
  Commit at(int idx);

  // void addComment(Comment c);
  // void deleteComment(int idx);
  // void deleteComment(Comment c);
};

} // namespace kan

#endif
