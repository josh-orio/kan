#ifndef CORE_BOARD_HPP
#define CORE_BOARD_HPP

#include <kan/core/column.hpp>
#include <nlohmann/json.hpp>
#include <vector>

namespace kan {

class Board {
private:
  std::string              _title;
  std::vector<kan::Column> _columns;

public:
  static Board &instance();

  Board() = default;
  Board(nlohmann::json board);

  void           fromJson(nlohmann::json json);
  nlohmann::json toJson();

  std::vector<std::string> Statuses();

  std::string          Title();
  std::vector<Column> &Columns();

  Column           &Column(unsigned int idx);
  std::vector<Job> &Jobs(unsigned int col_idx);
  Job              &Job(unsigned int col_idx, unsigned int row_idx);

  void deleteJob(kan::Job job);
};

} // namespace kan

#endif
