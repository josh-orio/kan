#ifndef CORE_COLUMN_HPP
#define CORE_COLUMN_HPP

#include <kan/core/job.hpp>
#include <string>
#include <vector>

namespace kan {

struct Column {
  std::string           title;
  std::vector<kan::Job> jobs;
  termui::Style         header = defaultHeader(); // how the title is drawn on the board, set in settings

  static termui::Style defaultHeader();

public:
  Column() = default;
  Column(YAML::Node yaml);

  void       fromYaml(YAML::Node yaml);
  YAML::Node toYaml();

  std::vector<kan::Job> &Jobs();
  kan::Job              &Job(unsigned int row_idx);
  void                   DeleteJob(kan::Job job);
};

} // namespace kan

#endif
