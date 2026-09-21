#include <kan/core.hpp>

namespace kan {

// // Column::Column(nlohmann::json json) : _title(json["title"]) {
Column::Column(YAML::Node yaml) { fromYaml(yaml); }

// // std::cout << board["columns"].dump(2) << std::endl<<std::endl;

// // for (auto col : board["columns"]) {
// //     std::cout << col.dump() <<std::endl;
// // }
// }

// // nlohmann::json Column::toJson() {
// //   nlohmann::json column;

// //   column["title"] = _title;

// //   return column;
// // }

// // void Column::fromJson(nlohmann::json json) {

// //   _title = json["title"];
// //   //   _description = json["description"];
// //   //   status = json["status"];
// //   //   created = json["created"];
// //   //   _updated = json["updated"];
// //   //   //    creator.toJson()=json["creator"];
// //   //   uuid = json["uuid"];
// // };

void Column::fromYaml(YAML::Node yaml) {
  title = yaml["title"].as<std::string>();
  // _description = yaml["description"];
  // status = yaml["status"];
  // created = yaml["created"];
  // _updated = yaml["updated"];
  // //    creator.toJson()=json["creator"];
  // uuid = yaml["uuid"];
}

YAML::Node Column::toYaml() {
  YAML::Node yaml;
  yaml["title"] = title;

  // std::vector<YAML::Node> col_jsons;
  // for (auto &c : columns) {
  //   col_jsons.push_back(c.toYaml());
  // }

  // yaml["columns"] = col_jsons;

  return yaml;
}

// // std::string kan::Column::Title() { return _title; }

std::vector<Job> &Column::Jobs() { return jobs; }

Job &Column::Job(unsigned int row_idx) { return jobs.at(row_idx); }

void Column::DeleteJob(kan::Job job) {
  jobs.erase(std::remove_if(jobs.begin(), jobs.end(), [&](const kan::Job &j) { return job.uuid == j.uuid; }), jobs.end());
}

} // namespace kan
