#ifndef CORE_GIT_HPP
#define CORE_GIT_HPP

#include <filesystem>
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

std::vector<std::string> fileDiff(const std::string &hash, const FileChange &file); // unified diff lines, headers stripped

// a local branch, read from git on demand
struct Branch {
  std::string name;
  std::string tip;             // commit hash the branch points at
  bool        current = false; // checked out
  bool        merged  = false; // has commits of its own and they're all in the default branch
};

std::vector<Branch> branches(); // local branches, empty outside a git repo

std::string defaultBranch(); // main, master or whatever origin/HEAD points at, empty if there's none

std::string jobBranchName(int id, const std::string &title); // 12, "Fix login crash!" -> 12-fix-login-crash
int         branchJobId(const std::string &branch);          // 12-fix-login-crash or josh/12-fix -> 12, 0 if it isn't a job branch

std::string switchBranch(const std::string &name, bool create); // git switch [-c], returns git's error or "" on success

std::filesystem::path gitDir();    // .git of this worktree, empty outside a git repo
std::filesystem::path commonDir(); // .git shared by every worktree - branches live here

} // namespace git
} // namespace kan

#endif
