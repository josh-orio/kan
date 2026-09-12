#include <kan/core.hpp>

namespace kan {

Board &Board::instance() {
  static Board board;
  return board;
}

Board::Board(nlohmann::json board) : _title(board["title"]) {
  for (auto col : board["columns"]) {
    _columns.push_back(Column(col));
  }
}

nlohmann::json Board::toJson() { return nlohmann::json{{"title", _title}, {}}; }

std::vector<std::string> Board::Statuses() {
  std::vector<std::string> statuses;

  for (auto &c : _columns) {
    statuses.push_back(c.Title());
  }

  return statuses;
}

void Board::fromJson(nlohmann::json json) {
  _title = json["title"];

  kan::Column column;
  for (auto col : json["columns"]) {
    column.fromJson(col);
    _columns.push_back(column);
  }
};

std::string Board::Title() { return _title; }

std::vector<Column>& Board::Columns() { return _columns; }

Column &Board::Column(unsigned int idx) { return _columns.at(idx); }

std::vector<Job> &Board::Jobs(unsigned int col_idx) { return _columns.at(col_idx).Jobs(); }

Job &Board::Job(unsigned int col_idx, unsigned int row_idx) { return _columns.at(col_idx).Job(row_idx); }

} // namespace kan
