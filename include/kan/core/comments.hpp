#ifndef CORE_COMMENTS_HPP
#define CORE_COMMENTS_HPP

#include <kan/core/timestamp.hpp>
#include <kan/core/user.hpp>
#include <kan/core/uuid.hpp>
#include <string>
#include <termui/termui.hpp>

namespace kan {

struct Comment {
  termui::string text;
  User           creator;
  Timestamp      created;
  UUID           uuid;

  Comment();
  Comment(YAML::Node yaml);

  void       fromYaml(YAML::Node yaml);
  YAML::Node toYaml();
};

struct Comments {
private:
  std::vector<Comment> _comments;

public:
  Comments();
  Comments(YAML::Node yaml);

  void       fromYaml(YAML::Node yaml);
  YAML::Node toYaml();

  std::vector<Comment>::iterator begin();
  std::vector<Comment>::iterator end();

  size_t  size();
  Comment at(int idx);

  void addComment(Comment c);
  void deleteComment(int idx);
  void deleteComment(Comment c);
};

} // namespace kan

#endif
