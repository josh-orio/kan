#include <kan/core.hpp>

namespace kan {

// ---- Comment ----
Comment::Comment() : uuid(UUID::generate()) {}

Comment::Comment(YAML::Node yaml) { fromYaml(yaml); }

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

Comments::Comments(YAML::Node yaml) { fromYaml(yaml); }

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

std::vector<Comment>::iterator Comments::begin() { return _comments.begin(); }

std::vector<Comment>::iterator Comments::end() { return _comments.end(); }

size_t Comments::size() { return _comments.size(); }

Comment Comments::at(int i) { return _comments.at(i); }

void Comments::addComment(Comment c) { _comments.push_back(c); }

void Comments::deleteComment(int idx) { _comments.erase(_comments.begin() + idx); }

void Comments::deleteComment(Comment c) {
  _comments.erase(std::remove_if(_comments.begin(), _comments.end(), [&](const kan::Comment &comment) { return comment.uuid == c.uuid; }), _comments.end());
}

} // namespace kan
