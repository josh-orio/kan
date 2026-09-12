#include <kan/core.hpp>

#include <ctime>
#include <format>
#include <sstream>
#include <string>

namespace kan {

Timestamp &Timestamp::fromJson(nlohmann::json timestamp) {
  time = timestamp.at("time");
  timezone = timestamp.at("timezone");

  return *this;
}

nlohmann::json Timestamp::toJson() { return {{"time", time}, {"timezone", timezone}}; }

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

  //   auto time = std::chrono::system_clock::from_time_t(timestamp);

  //     auto seconds_tp = std::chrono::floor<std::chrono::seconds>(time);

  //   std::string formatted = std::format("{:%d-%b-%Y %H:%M:%S}", seconds_tp);

  //   return formatted;

  // auto time = std::chrono::system_clock::to_time_t(seconds_tp);

  // std::tm tm{};
  // localtime_r(&timestamp, &tm);

  // char buffer[32];
  // std::strftime(buffer, sizeof(buffer), "%d-%b-%Y %H:%M:%S", &tm);

  // std::tm tm{};
  // if (localtime_r(&time, &tm) == nullptr) {
  //     throw std::runtime_error("localtime_r failed");
  // }

  // char buffer[32];

  // const auto length =
  //     std::strftime(buffer, sizeof(buffer), "%d-%b-%Y %H:%M:%S", &tm);

  // if (length == 0) {
  //     throw std::runtime_error("strftime failed");
  // }

  // std::string formatted(buffer);
}

} // namespace kan
