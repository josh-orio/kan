#ifndef CORE_COLUMN_HPP
#define CORE_COLUMN_HPP

#include <kan/core/job.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace kan {

class Column {
private:
  std::string      _title;
  std::vector<Job> _jobs;

public:
  Column() = default;
  Column(nlohmann::json json);

  nlohmann::json toJson();
  void           fromJson(nlohmann::json json);

  std::string       Title();
  std::vector<Job> &Jobs();
  Job              &Job(unsigned int row_idx);
  void              DeleteJob(kan::Job job);
};

} // namespace kan

#endif
