#include <iomanip>
#include <kan/core.hpp>

#include <ctime>
#include <format>
#include <sstream>
#include <string>

namespace kan {

Timestamp::Timestamp() { Now(); }

Timestamp::Timestamp(YAML::Node yaml) { fromYaml(yaml); }

Timestamp &Timestamp::fromYaml(YAML::Node yaml) {
  time     = yaml["time"].as<time_t>();
  timezone = yaml["timezone"].as<std::string>();

  return *this;
}

YAML::Node Timestamp::toYaml() {
  YAML::Node yaml;
  yaml["time"]     = time;
  yaml["timezone"] = timezone;

  return yaml;
}

Timestamp Timestamp::Now() {
  const auto clock_time = std::chrono::system_clock::now();

  time = std::chrono::duration_cast<std::chrono::seconds>(clock_time.time_since_epoch()).count();

  return *this;
}

std::string Timestamp::toString() {
  std::tm tm_buf;
  localtime_r(&time, &tm_buf); // or gmtime_r for UTC

  std::ostringstream oss;
  oss << std::put_time(&tm_buf, "%d %b %Y %H:%M:%S");
  return oss.str();
}

} // namespace kan
