#include <kan/core.hpp>

namespace kan {

Column::Column(YAML::Node yaml) { fromYaml(yaml); }

termui::Style Column::defaultHeader() { return termui::Styles::bold; }

void Column::fromYaml(YAML::Node yaml) {
  title  = yaml["title"].as<std::string>();
  header = styling::fromYaml(yaml["header"], defaultHeader());
}

YAML::Node Column::toYaml() {
  YAML::Node yaml;
  yaml["title"] = title;

  if (header != defaultHeader()) { // most columns keep the default, no need to spell it out
    yaml["header"] = styling::toYaml(header);
  }

  return yaml;
}

std::vector<Job> &Column::Jobs() { return jobs; }

Job &Column::Job(unsigned int row_idx) { return jobs.at(row_idx); }

void Column::DeleteJob(kan::Job job) {
  jobs.erase(std::remove_if(jobs.begin(), jobs.end(), [&](const kan::Job &j) { return job.uuid == j.uuid; }), jobs.end());
}

} // namespace kan
