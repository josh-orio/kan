#ifndef CORE_BOARD_HPP
#define CORE_BOARD_HPP

#include <filesystem>
#include <kan/core/column.hpp>
#include <vector>

namespace kan {

struct Board {
private:
  std::filesystem::path _path;

  void assignJobIds(std::vector<kan::Job> &jobs);

public:
  std::string              title;
  std::vector<kan::Column> columns;
  Issues                   issues;
  int                      next_id = 1;
  std::vector<User>        contributors; // people listed in contributors.yaml

  termui::StyleMap styleMap;

  static Board &instance();

  Board(std::string path);

  void       fromYaml(YAML::Node yaml);
  YAML::Node toYaml();

  std::vector<std::string> Statuses();
  std::vector<User>        Users();

  std::string          Title();
  std::vector<Column> &Columns();

  Column           &Column(unsigned int idx);
  std::vector<Job> &Jobs(unsigned int col_idx);
  Job              &Job(unsigned int col_idx, unsigned int row_idx);

  kan::Job *findJob(int id);
  int       nextJobId();

  void deleteJob(kan::Job job);

  void loadBoard();
  void saveBoard();

  void loadJobs();
  void saveJobs();

  void loadJob();
  void saveJob(uint col, uint row);
  void saveJob(kan::Job);

  void reloadJobs();

  void saveIssue(Issue issue);
  void loadIssues();

  void loadStyleMap();

  void loadContributors();
  bool isContributor(const User &user);
  void addContributor(User user);
  void saveContributors();
  void saveStyleMap();

  // Board&
  // void setTitle(std::string s);
  // void setDescription(std::string s);
  // void addColumn(std::string title);
};

} // namespace kan

#endif
