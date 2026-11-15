#ifndef CORE_COMMITS_HPP
#define CORE_COMMITS_HPP

#include <kan/core/timestamp.hpp>
#include <kan/core/user.hpp>
#include <kan/core/uuid.hpp>
#include <string>
#include <termui/termui.hpp>

namespace kan {

struct Commit {
  std::string hash;
  std::string message;
  User        creator;
  Timestamp   created;

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

  std::vector<Commit>::iterator begin();
  std::vector<Commit>::iterator end();

  size_t size();
  Commit at(int idx);

  void addCommit(Commit c);
  void deleteCommit(int idx);
  void deleteCommit(Commit c);
};

} // namespace kan

#endif
