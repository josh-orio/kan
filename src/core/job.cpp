#include <kan/core.hpp>

namespace kan {

Job::Job() : title(), description(), status(), created(), updated(), creator(), uuid(UUIDv7::generate().to_string()) {
  time_t timestamp;
  created = time(&timestamp);
  updated = created;

  creator = User().getFromSystem();
}

Job::Job(nlohmann::json json) { fromJson(json); }

nlohmann::json Job::toJson() {
  nlohmann::ordered_json job;
  job["title"] = title;
  job["description"] = description;
  job["status"] = status;
  job["created"] = created;
  job["updated"] = updated;
  job["creator"] = creator.toJson();
  job["uuid"] = uuid;

  job["comments"] = comments.toJson();

  // job

  return job;
};

void Job::fromJson(nlohmann::json json) {
  std::string tmp = json.dump();

  title = json["title"];
  description = json["description"];
  status = json["status"];
  created = json["created"];
  updated = json["updated"];
  //  creator.fromJson(json["creator"]);
  uuid = json["uuid"];

  if (json.contains("comments")) {
    std::string mosey = json.at("comments").dump(4);
  }

  comments = Comments(json.value("comments", nlohmann::json::object()));
  // _comments=Comments();

  //   std::string _title, _description, _status, uuid;

  // User   creator, _modifier;
  // time_t created, _updated;

  // Comments    _comments;
  // Issues      _issues;
  // std::vector<std::string> _history;
};

} // namespace kan
