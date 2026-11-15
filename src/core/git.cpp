#include <algorithm>
#include <cctype>
#include <charconv>
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

// runs a command, giving back its exit status and stdout
static std::pair<int, std::string> exec(const std::string &cmd) {
  FILE *pipe = popen(cmd.c_str(), "r");

  if (pipe == nullptr) {
    throw std::runtime_error("failed to run git");
  }

  std::string output;
  char        buffer[4096];

  while (size_t n = fread(buffer, 1, sizeof(buffer), pipe)) {
    output.append(buffer, n);
  }

  return {pclose(pipe), output};
}

static std::string run(const std::string &cmd) {
  auto [status, output] = exec(cmd);

  if (status != 0) {
    throw std::runtime_error("git failed: " + cmd);
  }

  return output;
}

static std::vector<std::string> lines(const std::string &output) {
  std::vector<std::string> out;
  size_t                   pos = 0;

  while (pos < output.size()) {
    size_t next = output.find('\n', pos);
    if (next == std::string::npos) {
      next = output.size();
    }

    out.push_back(output.substr(pos, next - pos));
    pos = next + 1;
  }

  return out;
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

// wraps a path in single quotes for the shell
static std::string quote(const std::string &s) {
  std::string out = "'";

  for (char c : s) {
    out += c == '\'' ? std::string("'\\''") : std::string(1, c);
  }

  return out + "'";
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

std::vector<std::string> fileDiff(const std::string &hash, const FileChange &file) {
  checkHash(hash);

  auto paths = quote(file.path);
  if (!file.old_path.empty()) {
    paths = quote(file.old_path) + " " + paths;
  }

  auto output = run(std::format("git diff-tree -p -M --root --no-commit-id --no-color --no-ext-diff {} -- {} 2>/dev/null", hash, paths));

  std::vector<std::string> lines;
  bool                     in_hunk = false; // skip the "diff --git", "index", "---", "+++" header lines
  size_t                   pos     = 0;

  while (pos < output.size()) {
    size_t next = output.find('\n', pos);
    if (next == std::string::npos) {
      next = output.size();
    }

    auto line = output.substr(pos, next - pos);
    pos       = next + 1;

    if (line.starts_with("@@")) {
      in_hunk = true;
    }
    else if (line.starts_with("diff --git")) {
      in_hunk = false;
    }

    if (!in_hunk) {
      if (line.starts_with("Binary files")) {
        lines.push_back(line);
      }
      continue;
    }

    // tabs would throw off column widths in the terminal
    for (size_t t = line.find('\t'); t != std::string::npos; t = line.find('\t', t)) {
      line.replace(t, 1, "  ");
    }

    lines.push_back(line);
  }

  return lines;
}

std::string defaultBranch() {
  auto exists = [](const std::string &ref) { return exec(std::format("git show-ref --verify --quiet {} 2>/dev/null", quote(ref))).first == 0; };

  // a clone knows the remote's default, prefer the local copy of it since that's what gets merged into first
  auto [status, origin] = exec("git symbolic-ref --quiet --short refs/remotes/origin/HEAD 2>/dev/null");
  origin.erase(origin.find_last_not_of("\n") + 1);

  if (status == 0 && origin.starts_with("origin/")) {
    auto name = origin.substr(7);
    return exists("refs/heads/" + name) ? name : origin;
  }

  for (std::string name : {"main", "master"}) {
    if (exists("refs/heads/" + name)) {
      return name;
    }
  }

  return "";
}

std::vector<Branch> branches() {
  auto [status, output] = exec("git for-each-ref --format='%(refname:short)%00%(objectname)%00%(HEAD)' refs/heads 2>/dev/null");

  if (status != 0) {
    return {};
  }

  std::vector<Branch> out;

  for (const auto &line : lines(output)) {
    auto parts = fields(line);

    if (parts.size() < 3) {
      continue;
    }

    out.push_back({parts[0], parts[1], parts[2] == "*"});
  }

  auto target = defaultBranch();

  if (target.empty()) {
    return out;
  }

  auto [merged_status, merged_output] = exec(std::format("git for-each-ref --merged {} --format='%(refname:short)' refs/heads 2>/dev/null", quote(target)));

  if (merged_status != 0) {
    return out;
  }

  auto merged = lines(merged_output);

  auto [tip_status, target_tip] = exec(std::format("git rev-parse {} 2>/dev/null", quote(target)));
  target_tip.erase(target_tip.find_last_not_of("\n") + 1);

  for (auto &branch : out) {
    if (branch.name == target || branchJobId(branch.name) == 0 || std::find(merged.begin(), merged.end(), branch.name) == merged.end()) {
      continue;
    }

    // a branch that was just created is "merged" too, as nothing is on it yet. the reflog tells them apart -
    // one entry means it hasn't moved since it was created. only job branches are checked, it's a git call each
    auto [reflog_status, reflog] = exec(std::format("git reflog show --format=%H {} -- 2>/dev/null", quote("refs/heads/" + branch.name)));
    auto moves                   = lines(reflog).size();

    if (reflog_status == 0 && moves > 0) {
      branch.merged = moves > 1;
    }
    else { // no reflog, eg a branch that came from a fetch - best guess
      branch.merged = branch.tip != target_tip;
    }
  }

  return out;
}

std::string jobBranchName(int id, const std::string &title) {
  std::string slug;
  bool        gap = false;

  for (unsigned char c : title) {
    if (std::isalnum(c) && c < 128) {
      if (gap && !slug.empty()) {
        slug += '-';
      }
      slug += static_cast<char>(std::tolower(c));
      gap = false;
    }
    else {
      gap = true;
    }
  }

  // keep it short, cutting at a word break
  constexpr size_t max = 40;

  if (slug.size() > max) {
    auto cut = slug.rfind('-', max);
    slug     = slug.substr(0, cut == std::string::npos ? max : cut);
  }

  return slug.empty() ? std::to_string(id) : std::format("{}-{}", id, slug);
}

int branchJobId(const std::string &branch) {
  auto name = branch.substr(branch.rfind('/') + 1); // josh/12-fix -> 12-fix

  int  id = 0;
  auto [ptr, ec] = std::from_chars(name.data(), name.data() + name.size(), id);

  if (ec != std::errc() || ptr == name.data() || id <= 0) {
    return 0;
  }

  return ptr == name.data() + name.size() || *ptr == '-' ? id : 0;
}

std::string switchBranch(const std::string &name, bool create) {
  auto [status, output] = exec(std::format("git switch {}{} 2>&1", create ? "-c " : "", quote(name)));

  if (status == 0) {
    return "";
  }

  output.erase(output.find_last_not_of("\n") + 1);
  return output.empty() ? std::string("git switch failed") : output;
}

static std::filesystem::path revParse(const std::string &args) {
  auto [status, output] = exec(std::format("git rev-parse {} 2>/dev/null", args));
  output.erase(output.find_last_not_of("\n") + 1);

  return status == 0 ? std::filesystem::path(output) : std::filesystem::path();
}

std::filesystem::path gitDir() { return revParse("--absolute-git-dir"); }

std::filesystem::path commonDir() {
  auto dir = revParse("--path-format=absolute --git-common-dir"); // git 2.31+
  return dir.empty() ? gitDir() : dir;
}

} // namespace git
} // namespace kan
