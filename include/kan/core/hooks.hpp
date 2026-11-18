#ifndef CORE_HOOKS_HPP
#define CORE_HOOKS_HPP

#include <kan/core/board.hpp>
#include <string>
#include <vector>

namespace kan {
namespace hooks {

std::vector<int> parseJobRefs(const std::string &message); // ids referenced like #12, in order, without repeats

std::vector<int> parseClosingRefs(const std::string &message); // ids after fixes/closes/finishes/resolves (any tense), e.g. "fixes #12, #13"

void postCommit(Board &board, const std::string &hash);

// adds kan's post-commit hook to the git repo holding <repo>, keeping whatever hook is already there.
// returns what happened, for printing ("" when there's nothing to say, eg outside a git repo or already installed)
std::string installHook(const std::filesystem::path &repo);

} // namespace hooks
} // namespace kan

#endif
