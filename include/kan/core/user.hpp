#ifndef CORE_USER_HPP
#define CORE_USER_HPP

#include <nlohmann/json.hpp>
#include <string>

namespace kan {

struct User { // struct to represent a Git user
  std::string name, email;

  User &from_json(nlohmann::json user);

  nlohmann::json to_json();

  User &get_from_system();

  std::string toString();
  std::string Name();
};

} // namespace kan

#endif
