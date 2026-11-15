#ifndef CORE_USER_HPP
#define CORE_USER_HPP

#include <string>
#include <yaml-cpp/yaml.h>

namespace kan {

struct User { // struct to represent a Git user
  std::string name, email;

  User      &fromYaml(YAML::Node yaml);
  YAML::Node toYaml();

  User &getFromSystem();

  std::string toString();
  std::string Name();
  
  bool operator==(const User& other) const;
};

} // namespace kan

#endif
