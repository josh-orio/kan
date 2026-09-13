#include <kan/core.hpp>

namespace kan {

// ---- Comment ----
Comment::Comment() {}

Comment::Comment(nlohmann::json json) { fromJson(json); }

void Comment::fromJson(nlohmann::json comment) {

  text    = comment.at("text");
  creator = User().fromJson(comment.at("creator"));
  created = Timestamp().fromJson(comment.at("created"));
  uuid    = comment.at("uuid");
}

nlohmann::json Comment::toJson() {
  return {
      {"text", text},
      {"creator", creator.toJson()},
      {"created", created.toJson()},
      {"uuid", uuid},
  };
}

// ---- Comments ----
Comments::Comments() : _comments() {}

Comments::Comments(nlohmann::json json) { fromJson(json); }

void Comments::fromJson(nlohmann::json comments) {
  _comments.clear();

  if (comments.empty() || comments.is_null()) {
    return;
  }

  for (auto &c : comments) {
    _comments.push_back(Comment(c));
  }
}

nlohmann::json Comments::toJson() {
  auto json = nlohmann::json::array();

  for (auto &c : _comments) {
    json.push_back(c.toJson());
  }

  return json;
}

size_t Comments::size() { return _comments.size(); }

Comment Comments::at(int i) { return _comments.at(i); }

void Comments::addComment(Comment c) { _comments.push_back(c); }

void Comments::deleteComment(int idx) { _comments.erase(_comments.begin() + idx); }

void Comments::deleteComment(Comment c) {
  _comments.erase(std::remove_if(_comments.begin(), _comments.end(), [&](const kan::Comment &comment) { return comment.uuid == c.uuid; }), _comments.end());
}

} // namespace kan
