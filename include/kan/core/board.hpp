#ifndef CORE_BOARD_HPP
#define CORE_BOARD_HPP

#include <filesystem>
#include <kan/core/column.hpp>
// #include <nlohmann/json.hpp>
#include <vector>

namespace kan {

struct Board {
  private:
   std::filesystem::path  _path;

public:
  std::string              title;
  std::vector<kan::Column> columns;
  Issues                   issues;

  static Board &instance();

  // Board() = default;
  Board(std::string path);
  // Board(nlohmann::json board);

  // void           fromJson(nlohmann::json json);
  // nlohmann::json toJson();

  void       fromYaml(YAML::Node yaml);
  YAML::Node toYaml();

  std::vector<std::string> Statuses();

  std::string          Title();
  std::vector<Column> &Columns();

  Column           &Column(unsigned int idx);
  std::vector<Job> &Jobs(unsigned int col_idx);
  Job              &Job(unsigned int col_idx, unsigned int row_idx);

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

  // Board&
  // void setTitle(std::string s);
  // void setDescription(std::string s);
  // void addColumn(std::string title);
};

} // namespace kan

#endif
