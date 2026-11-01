#ifndef CORE_HOOKS_HPP
#define CORE_HOOKS_HPP

#include <kan/core/board.hpp>
#include <string>
#include <vector>

namespace kan {
namespace hooks {

std::vector<int> parseJobRefs(const std::string &message); // ids referenced like #12, in order, without repeats

void postCommit(Board &board, const std::string &hash);

} // namespace hooks
} // namespace kan

#endif
