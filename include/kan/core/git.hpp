#ifndef CORE_GIT_HPP
#define CORE_GIT_HPP

#include <kan/core/commits.hpp>
#include <string>
#include <vector>

namespace kan {
namespace git {

// a file touched by a commit, read from git on demand (never stored in .kan)
struct FileChange {
  char        status;   // A added, M modified, D deleted, R renamed, C copied, T type changed
  std::string path;     // path after the commit
  std::string old_path; // path before the commit, only set for renames/copies
  int         additions = 0, deletions = 0;
  bool        binary    = false; // git doesn't count lines for binary files
};

Commit readCommit(const std::string &hash);

std::vector<FileChange> changedFiles(const std::string &hash); // empty for merge commits

} // namespace git
} // namespace kan

#endif
