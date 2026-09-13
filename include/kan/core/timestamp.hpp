#ifndef CORE_TIMESTAMP_HPP
#define CORE_TIMESTAMP_HPP

#include <nlohmann/json.hpp>

namespace kan {

struct Timestamp {
  time_t      time; // this must always be in utc, then timezone can be used to decode into local time
  std::string timezone;

  Timestamp();
  Timestamp(nlohmann::json json);

  Timestamp     &fromJson(nlohmann::json timestamp);
  nlohmann::json toJson();

  Timestamp Now();

  std::string toString();
};

} // namespace kan

#endif
