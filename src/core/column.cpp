#include <kan/core.hpp>

namespace kan {

Column::Column(nlohmann::json json) : _title(json["title"]) {

  // std::cout << board["columns"].dump(2) << std::endl<<std::endl;

  // for (auto col : board["columns"]) {
  //     std::cout << col.dump() <<std::endl;
  // }
}

// nlohmann::json Board::toJson() {
//     return nlohmann::json {
//         {"title", _title},
//         {}
//     };
// }

void Column::fromJson(nlohmann::json json) {

  _title = json["title"];
  //   _description = json["description"];
  //   status = json["status"];
  //   created = json["created"];
  //   _updated = json["updated"];
  //   //    creator.toJson()=json["creator"];
  //   uuid = json["uuid"];
};

std::string Column::Title() { return _title; }

std::vector<Job> &Column::Jobs() { return _jobs; }

Job &Column::Job(unsigned int row_idx) { return _jobs.at(row_idx); }

} // namespace kan
