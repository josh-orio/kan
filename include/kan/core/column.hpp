#ifndef CORE_COLUMN_HPP
#define CORE_COLUMN_HPP

#include <kan/core/job.hpp>
// #include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace kan {

struct Column {
  // private:
  std::string      title;
  std::vector<Job> jobs;

public:
  Column() = default;
  // Column(nlohmann::json json);
  Column(YAML::Node yaml);

  void       fromYaml(YAML::Node yaml);
  YAML::Node toYaml();

  // nlohmann::json toJson();
  // void           fromJson(nlohmann::json json);

  // std::string       Title();
  std::vector<Job> &Jobs();
  Job              &Job(unsigned int row_idx);
  void              DeleteJob(kan::Job job);
};

} // namespace kan

#endif
