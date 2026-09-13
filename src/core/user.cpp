#include <kan/core.hpp>

namespace kan {

User &User::fromJson(nlohmann::json user) {
  name  = user.at("name");
  email = user.at("email");

  return *this;
};

nlohmann::json User::toJson() {
  nlohmann::ordered_json user;
  user["name"]  = name;
  user["email"] = email;

  return user;
};

std::string runCommand(const std::string &cmd) {
  std::array<char, 256> buffer;
  std::string           result;

  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
  if (!pipe) {
    throw std::runtime_error("popen() failed");
  }

  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    result += buffer.data();
  }

  // strip trailing newline
  while (!result.empty() && (result.back() == '\n' || result.back() == '\r')) {
    result.pop_back();
  }

  return result;
};

User &User::getFromSystem() {
  name  = runCommand("git config user.name 2>/dev/null");
  email = runCommand("git config user.email 2>/dev/null");

  return *this;
};

std::string User::toString() { return std::format("{} ({})", name, email); }

std::string User::Name() { return std::format("{}", name); }

} // namespace kan
