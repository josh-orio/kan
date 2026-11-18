#include <algorithm>
#include <cctype>
#include <charconv>
#include <cstdio>
#include <format>
#include <fstream>
#include <iostream>
#include <kan/core.hpp>
#include <stdexcept>

namespace kan {
namespace hooks {

static bool isWordChar(char c) { return std::isalnum(static_cast<unsigned char>(c)) || c == '_'; }

std::vector<int> parseJobRefs(const std::string &message) {
  std::vector<int> ids;

  for (size_t pos = message.find('#'); pos != std::string::npos; pos = message.find('#', pos + 1)) {
    if (pos > 0 && isWordChar(message[pos - 1])) { // abc#12 isn't a reference
      continue;
    }

    const char *start = message.data() + pos + 1;
    const char *end   = message.data() + message.size();

    int id         = 0;
    auto [ptr, ec] = std::from_chars(start, end, id);

    if (ec != std::errc() || ptr == start || (ptr != end && isWordChar(*ptr)) || id <= 0) { // neither is #12abc
      continue;
    }

    if (std::find(ids.begin(), ids.end(), id) == ids.end()) {
      ids.push_back(id);
    }
  }

  return ids;
}

std::vector<int> parseClosingRefs(const std::string &message) {
  static const std::vector<std::string> keywords = {"fix",    "fixes",    "fixed",    "close",   "closes",   "closed",
                                                    "finish", "finishes", "finished", "resolve", "resolves", "resolved"};

  std::string lower = message;
  std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return std::tolower(c); });

  std::vector<int> ids;

  for (size_t pos = 0; pos < lower.size();) {
    // read a word
    if (!std::isalpha(static_cast<unsigned char>(lower[pos]))) {
      pos++;
      continue;
    }

    size_t end = pos;
    while (end < lower.size() && std::isalpha(static_cast<unsigned char>(lower[end]))) {
      end++;
    }

    bool keyword = (pos == 0 || !isWordChar(lower[pos - 1])) && std::find(keywords.begin(), keywords.end(), lower.substr(pos, end - pos)) != keywords.end();
    pos          = end;

    if (!keyword) {
      continue;
    }

    // "fixes #1", "fixes: #1", "fixes #1, #2 and #3" - refs separated by spaces, commas, colons or "and"
    while (true) {
      size_t p = pos;
      while (p < lower.size() && (lower[p] == ' ' || lower[p] == ',' || lower[p] == ':')) {
        p++;
      }
      if (lower.compare(p, 4, "and ") == 0) {
        p += 4;
        while (p < lower.size() && lower[p] == ' ') {
          p++;
        }
      }

      if (p >= lower.size() || lower[p] != '#') {
        break;
      }

      auto refs = parseJobRefs(message.substr(p, message.find_first_of(" ,:\n", p) - p));
      if (refs.empty()) {
        break;
      }

      if (std::find(ids.begin(), ids.end(), refs.front()) == ids.end()) {
        ids.push_back(refs.front());
      }

      pos = message.find_first_of(" ,:\n", p);
      if (pos == std::string::npos) {
        pos = lower.size();
        break;
      }
    }
  }

  return ids;
}

void postCommit(Board &board, const std::string &hash) {
  Commit commit = git::readCommit(hash);

  for (int id : parseJobRefs(commit.message)) {
    Job *job = board.findJob(id);

    if (job == nullptr) {
      std::cerr << std::format("kan: no job #{} to link\n", id);
      continue;
    }

    bool linked = std::any_of(job->commits.begin(), job->commits.end(), [&](const Commit &c) { return c.hash == commit.hash; });

    if (linked) {
      continue;
    }

    job->commits.addCommit(commit);
    job->addEvent({std::format("Commit {} linked.", commit.hash.substr(0, 7)), commit.creator, commit.created}, false);

    board.saveJob(*job);

    std::cout << std::format("kan: linked commit to #{}\n", id);
  }

  for (int id : parseClosingRefs(commit.message)) {
    Job *job = board.findJob(id);

    if (job == nullptr || job->finished || job->archived) {
      continue;
    }

    job->finish(commit.creator, commit.created, std::format("Job finished by commit {}.", commit.hash.substr(0, 7)));
    board.saveJob(*job);

    std::cout << std::format("kan: finished #{}\n", id);
  }
}

std::string installHook(const std::filesystem::path &repo) {
  static const std::string block = "\n# kan - links commits that mention #N to job N (see kan help)\n"
                                   "command -v kan >/dev/null 2>&1 && kan link-commit \"$(git rev-parse HEAD)\"\n";

  auto dir = git::hooksDir(repo);
  if (dir.empty()) {
    return "";
  }

  auto hook = dir / "post-commit";

  std::string existing;
  if (std::filesystem::exists(hook)) {
    std::ifstream in(hook);
    existing.assign(std::istreambuf_iterator<char>(in), {});
  }

  if (existing.find("kan link-commit") != std::string::npos) {
    return "";
  }

  // appending shell to a python (or any other) hook would break it
  if (!existing.empty() && existing.substr(0, existing.find('\n')).find("sh") == std::string::npos) {
    return std::format(
        "kan: {} isn't a shell script, so commits won't be linked to jobs. Add this line to it to link them:\n  kan link-commit \"$(git rev-parse HEAD)\"",
        hook.string());
  }

  std::error_code ec;
  std::filesystem::create_directories(dir, ec);

  std::ofstream(hook, std::ios::app) << (existing.empty() ? "#!/bin/sh\n" : "") << block;

  std::filesystem::permissions(hook, std::filesystem::perms::owner_exec | std::filesystem::perms::group_exec | std::filesystem::perms::others_exec,
                               std::filesystem::perm_options::add, ec);

  return std::format("kan: added a post-commit hook ({}) so commits that mention #N are linked to job N", hook.string());
}

} // namespace hooks
} // namespace kan
