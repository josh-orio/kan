#ifndef CORE_COMMENTS_HPP
#define CORE_COMMENTS_HPP

#include <kan/core/uuid.hpp>
#include <kan/core/timestamp.hpp>
#include <kan/core/user.hpp>
// #include <nlohmann/json.hpp>
#include <string>
#include <termui/termui.hpp>

namespace kan {

struct Comment {
  termui::string text; // std::string text;
  User           creator;
  Timestamp      created;
  UUID           uuid;

  Comment();
  // Comment(nlohmann::json json);
  Comment(YAML::Node yaml);

  // void           fromJson(nlohmann::json comment);
  // nlohmann::json toJson();

  void       fromYaml(YAML::Node yaml);
  YAML::Node toYaml();
};

struct Comments {
private:
  std::vector<Comment> _comments;

public:
  Comments();
  // Comments(nlohmann::json json);
  Comments(YAML::Node yaml);

  // void           fromJson(nlohmann::json comment);
  // nlohmann::json toJson();

  void       fromYaml(YAML::Node yaml);
  YAML::Node toYaml();

  size_t  size();
  Comment at(int idx);

  void addComment(Comment c);
  void deleteComment(int idx);
  void deleteComment(Comment c);
};

} // namespace kan

#endif
