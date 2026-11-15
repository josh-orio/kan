#ifndef KAN_CLI_HPP
#define KAN_CLI_HPP

#include <string>
#include <vector>

namespace kan {
namespace cli {

// kan new / ls / show / mv ... - the board from the command line, for scripts, CI and git aliases.
// these never touch the terminal UI, so they work without a terminal

bool isCommand(const std::string &arg);

int run(const std::vector<std::string> &args); // args[0] is the command, returns the exit status

int matchColumn(const std::vector<std::string> &columns, const std::string &name); // case-insensitive, a unique prefix will do. throws if nothing fits

} // namespace cli
} // namespace kan

#endif
