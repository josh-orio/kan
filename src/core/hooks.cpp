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
    job->addEvent({std::format("Commit {} linked.", commit.hash.substr(0, 7)), commit.creator, commit.created});

    board.saveJob(*job);

    std::cout << std::format("kan: linked commit to #{}\n", id);
  }
}

} // namespace hooks
} // namespace kan
