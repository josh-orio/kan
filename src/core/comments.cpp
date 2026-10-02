#include <kan/core.hpp>

namespace kan {

// ---- Comment ----
Comment::Comment() : uuid(UUID::generate()) {}

// Comment::Comment(nlohmann::json json) { fromJson(json); }

Comment::Comment(YAML::Node yaml) { fromYaml(yaml); }

// void Comment::fromJson(nlohmann::json comment) {
//   text    = termui::string(comment.at("text"));
//   creator = User().fromJson(comment.at("creator"));
//   created = Timestamp().fromJson(comment.at("created"));
//   uuid    = comment.at("uuid");
// }

// nlohmann::json Comment::toJson() {
//   return {
//       {"text", text},
//       {"creator", creator.toJson()},
//       {"created", created.toJson()},
//       {"uuid", uuid},
//   };
// }

void Comment::fromYaml(YAML::Node yaml) {
  text    = yaml["text"].as<std::string>();
  creator = User().fromYaml(yaml["creator"]);
  created = Timestamp().fromYaml(yaml["created"]);
  uuid    = UUID::from_string(yaml["uuid"].as<std::string>());
}

YAML::Node Comment::toYaml() {
  YAML::Node yaml;
  yaml["text"]    = std::string(text);
  yaml["creator"] = creator.toYaml();
  yaml["created"] = created.toYaml();
  yaml["uuid"]    = uuid.to_string();

  return yaml;
}

// ---- Comments ----
Comments::Comments() : _comments() {}

// Comments::Comments(nlohmann::json json) { fromJson(json); }

Comments::Comments(YAML::Node yaml) { fromYaml(yaml); }

// void Comments::fromJson(nlohmann::json comments) {
//   _comments.clear();

//   if (comments.empty() || comments.is_null()) {
//     return;
//   }

//   for (auto &c : comments) {
//     _comments.push_back(Comment(c));
//   }
// }

// nlohmann::json Comments::toJson() {
//   auto json = nlohmann::json::array();

//   for (auto &c : _comments) {
//     json.push_back(c.toJson());
//   }

//   return json;
// }

void Comments::fromYaml(YAML::Node yaml) {
  _comments.clear();

  for (const auto &c : yaml) {
    _comments.push_back(Comment(c));
  }
}

YAML::Node Comments::toYaml() {
  YAML::Node yaml(YAML::NodeType::Sequence);

  for (auto &e : _comments) {
    yaml.push_back(e.toYaml());
  }

  return yaml;
}

size_t Comments::size() { return _comments.size(); }

Comment Comments::at(int i) { return _comments.at(i); }

void Comments::addComment(Comment c) { _comments.push_back(c); }

void Comments::deleteComment(int idx) { _comments.erase(_comments.begin() + idx); }

void Comments::deleteComment(Comment c) {
  _comments.erase(std::remove_if(_comments.begin(), _comments.end(), [&](const kan::Comment &comment) { return comment.uuid == c.uuid; }), _comments.end());
}

} // namespace kan
