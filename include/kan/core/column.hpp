#ifndef CORE_COLUMN_HPP
#define CORE_COLUMN_HPP

#include <kan/core/job.hpp>
#include <string>
#include <vector>

namespace kan {

struct Column {
  std::string      title;
  std::vector<Job> jobs;

public:
  Column() = default;
  Column(YAML::Node yaml);

  void       fromYaml(YAML::Node yaml);
  YAML::Node toYaml();

  std::vector<Job> &Jobs();
  Job              &Job(unsigned int row_idx);
  void              DeleteJob(kan::Job job);
};

} // namespace kan

#endif
