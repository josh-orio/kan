#include <cstdio>
#include <format>
#include <kan/core.hpp>
#include <stdexcept>

namespace kan {
namespace git {

static void checkHash(const std::string &hash) {
  if (hash.empty() || hash.find_first_not_of("0123456789abcdef") != std::string::npos) { // this goes through a shell
    throw std::invalid_argument("not a commit hash: " + hash);
  }
}

static std::string run(const std::string &cmd) {
  FILE *pipe = popen(cmd.c_str(), "r");

  if (pipe == nullptr) {
    throw std::runtime_error("failed to run git");
  }

  std::string output;
  char        buffer[4096];

  while (size_t n = fread(buffer, 1, sizeof(buffer), pipe)) {
    output.append(buffer, n);
  }

  if (pclose(pipe) != 0) {
    throw std::runtime_error("git failed: " + cmd);
  }

  return output;
}

// splits git's -z output on NULs
static std::vector<std::string> fields(const std::string &output) {
  std::vector<std::string> out;
  size_t                   pos = 0;

  while (pos < output.size()) {
    size_t next = output.find('\0', pos);

    if (next == std::string::npos) {
      next = output.size();
    }

    out.push_back(output.substr(pos, next - pos));
    pos = next + 1;
  }

  return out;
}

Commit readCommit(const std::string &hash) {
  checkHash(hash);

  auto output = run(std::format("git log -1 --format=%H%x00%an%x00%ae%x00%at%x00%B {} 2>/dev/null", hash));

  std::vector<std::string> parts;
  size_t                   pos = 0;

  for (int i = 0; i < 4; i++) {
    size_t next = output.find('\0', pos);

    if (next == std::string::npos) {
      throw std::runtime_error("unexpected output from git");
    }

    parts.push_back(output.substr(pos, next - pos));
    pos = next + 1;
  }

  Commit commit;
  commit.hash          = parts[0];
  commit.creator.name  = parts[1];
  commit.creator.email = parts[2];
  commit.created.time  = std::stoll(parts[3]);
  commit.message       = output.substr(pos);

  commit.message.erase(commit.message.find_last_not_of('\n') + 1);

  return commit;
}

std::vector<FileChange> changedFiles(const std::string &hash) {
  checkHash(hash);

  // --root so the first commit lists its files too
  auto status = fields(run(std::format("git diff-tree --no-commit-id -r -M --root -z --name-status {} 2>/dev/null", hash)));
  auto counts = fields(run(std::format("git diff-tree --no-commit-id -r -M --root -z --numstat {} 2>/dev/null", hash)));

  std::vector<FileChange> files;

  // name-status: "M\0path\0" or "R100\0old\0new\0"
  for (size_t i = 0; i < status.size();) {
    FileChange f;
    f.status = status[i].empty() ? '?' : status[i][0];

    if ((f.status == 'R' || f.status == 'C') && i + 2 < status.size()) {
      f.old_path = status[i + 1];
      f.path     = status[i + 2];
      i += 3;
    }
    else if (i + 1 < status.size()) {
      f.path = status[i + 1];
      i += 2;
    }
    else {
      break;
    }

    files.push_back(f);
  }

  // numstat lists files in the same order: "adds\tdels\tpath\0" or "adds\tdels\t\0old\0new\0"
  size_t file = 0;

  for (size_t i = 0; i < counts.size() && file < files.size(); i++, file++) {
    auto &line = counts[i];
    auto  tab1 = line.find('\t'), tab2 = line.find('\t', tab1 + 1);

    if (tab1 == std::string::npos || tab2 == std::string::npos) {
      continue;
    }

    auto adds = line.substr(0, tab1), dels = line.substr(tab1 + 1, tab2 - tab1 - 1);

    if (adds == "-") {
      files[file].binary = true;
    }
    else {
      files[file].additions = std::stoi(adds);
      files[file].deletions = std::stoi(dels);
    }

    if (tab2 + 1 == line.size()) { // rename/copy - the two paths follow as their own fields
      i += 2;
    }
  }

  return files;
}

} // namespace git
} // namespace kan
