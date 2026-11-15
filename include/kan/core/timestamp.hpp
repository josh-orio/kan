#ifndef CORE_TIMESTAMP_HPP
#define CORE_TIMESTAMP_HPP

#include <yaml-cpp/yaml.h>

namespace kan {

struct Timestamp {
  time_t      time; // this must always be in utc, then timezone can be used to decode into local time
  std::string timezone;

  Timestamp();
  Timestamp(YAML::Node yaml);

  Timestamp &fromYaml(YAML::Node yaml);
  YAML::Node toYaml();

  Timestamp Now();

  std::string toString();
};

} // namespace kan

#endif
