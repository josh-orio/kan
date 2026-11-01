#include <algorithm>
#include <cctype>
#include <charconv>
#include <cstdio>
#include <format>
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

    int  id = 0;
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

static Commit readCommit(const std::string &hash) {
  if (hash.empty() || hash.find_first_not_of("0123456789abcdef") != std::string::npos) { // this goes through a shell
    throw std::invalid_argument("not a commit hash: " + hash);
  }

  FILE *pipe = popen(std::format("git log -1 --format=%H%x00%an%x00%ae%x00%at%x00%B {}", hash).c_str(), "r");

  if (pipe == nullptr) {
    throw std::runtime_error("failed to run git");
  }

  std::string output;
  char        buffer[4096];

  while (size_t n = fread(buffer, 1, sizeof(buffer), pipe)) {
    output.append(buffer, n);
  }

  if (pclose(pipe) != 0) {
    throw std::runtime_error("git could not read commit: " + hash);
  }

  std::vector<std::string> fields;
  size_t                   pos = 0;

  for (int i = 0; i < 4; i++) {
    size_t next = output.find('\0', pos);

    if (next == std::string::npos) {
      throw std::runtime_error("unexpected output from git");
    }

    fields.push_back(output.substr(pos, next - pos));
    pos = next + 1;
  }

  Commit commit;
  commit.hash          = fields[0];
  commit.creator.name  = fields[1];
  commit.creator.email = fields[2];
  commit.created.time  = std::stoll(fields[3]);
  commit.message       = output.substr(pos);

  commit.message.erase(commit.message.find_last_not_of('\n') + 1);

  return commit;
}

void postCommit(Board &board, const std::string &hash) {
  Commit commit = readCommit(hash);

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
    job->addEvent({std::format("Commit {} linked.", commit.hash.substr(0, 7)), commit.creator, commit.created});

    board.saveJob(*job);

    std::cout << std::format("kan: linked commit to #{}\n", id);
  }
}

} // namespace hooks
} // namespace kan
