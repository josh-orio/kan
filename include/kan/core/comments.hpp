#ifndef CORE_COMMENTS_HPP
#define CORE_COMMENTS_HPP

#include <kan/core/timestamp.hpp>
#include <kan/core/user.hpp>
#include <nlohmann/json.hpp>
#include <string>

namespace kan {

struct Comment {
  std::string text;
  User        creator;
  Timestamp   created;
  std::string uuid;

  Comment();
  Comment(nlohmann::json json);

  void           fromJson(nlohmann::json comment);
  nlohmann::json toJson();
};

struct Comments {
private:
  std::vector<Comment> _comments;

public:
  Comments();
  Comments(nlohmann::json json);

  void           fromJson(nlohmann::json comment);
  nlohmann::json toJson();

  size_t  size();
  Comment at(int idx);

  void addComment(Comment c);
  void deleteComment(int idx);
  void deleteComment(Comment c);
};

} // namespace kan

#endif
