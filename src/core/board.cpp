#include <filesystem>
#include <format>
#include <fstream>
#include <kan/core.hpp>
#include <set>

namespace kan {

Board &Board::instance() {
  static Board board = Board("");
  return board;
}

Board::Board(std::string path) : _path(path) {}

void Board::fromYaml(YAML::Node yaml) {
  title   = yaml["title"].as<std::string>();
  next_id = yaml["next_id"].as<int>(1);

  for (const auto &col : yaml["columns"]) {
    columns.push_back(kan::Column(col));
  }
}

YAML::Node Board::toYaml() {
  YAML::Node yaml;
  yaml["title"]   = title;
  yaml["next_id"] = next_id;

  std::vector<YAML::Node> col_jsons;
  for (auto &c : columns) {
    col_jsons.push_back(c.toYaml());
  }

  yaml["columns"] = col_jsons;

  return yaml;
}

std::vector<std::string> Board::Statuses() {
  std::vector<std::string> statuses;

  for (auto &c : columns) {
    statuses.push_back(c.title);
  }

  return statuses;
}

std::string Board::Title() { return title; }

std::vector<Column> &Board::Columns() { return columns; }

Column &Board::Column(unsigned int idx) { return columns.at(idx); }

std::vector<Job> &Board::Jobs(unsigned int col_idx) { return columns.at(col_idx).Jobs(); }

Job &Board::Job(unsigned int col_idx, unsigned int row_idx) { return columns.at(col_idx).Job(row_idx); }

Job *Board::findJob(int id) {
  for (auto &col : columns) {
    for (auto &job : col.Jobs()) {
      if (job.id == id) {
        return &job;
      }
    }
  }

  return nullptr;
}

int Board::nextJobId() {
  int id = next_id++;
  saveBoard();

  return id;
}

void Board::deleteJob(kan::Job job) {

  for (auto &col : board.Columns()) {
    auto &jobs = col.Jobs();
    jobs.erase(std::remove_if(jobs.begin(), jobs.end(), [&](const kan::Job &j) { return j.uuid == job.uuid; }), jobs.end());
  }

}

void Board::loadBoard() {

  YAML::Node config = YAML::LoadFile(_path / "kan.yaml");
  board.fromYaml(config);
}

void Board::saveBoard() {

  YAML::Emitter out;
  out << toYaml();

  std::ofstream fout(_path / "kan.yaml");
  fout << out.c_str();
}

// jobs from before ids existed get numbered here, and so do duplicates, which happen when
// two branches create the same id and get merged
void Board::assignJobIds(std::vector<kan::Job> &jobs) {
  int start_id = next_id;

  // uuids are v7 so this is oldest first, meaning the oldest job keeps a duplicated id
  std::sort(jobs.begin(), jobs.end(), [](const kan::Job &a, const kan::Job &b) { return a.uuid < b.uuid; });

  for (const auto &job : jobs) {
    next_id = std::max(next_id, job.id + 1);
  }

  std::set<int> used;

  for (auto &job : jobs) {
    if (job.id <= 0 || !used.insert(job.id).second) {
      job.id = next_id++;
      used.insert(job.id);

      saveJob(job);
    }
  }

  if (next_id != start_id) {
    saveBoard();
  }
}

void Board::loadJobs() {
  std::vector<kan::Job> loaded_jobs;

  for (const auto &entry : std::filesystem::directory_iterator(_path / "jobs")) {

    YAML::Node config = YAML::LoadFile(entry.path());
    loaded_jobs.push_back(kan::Job(config));

  }

  assignJobIds(loaded_jobs);

  for (const auto &loaded_job : loaded_jobs) {
    for (auto &col : board.Columns()) {
      if (col.title == loaded_job.status) {
        col.Jobs().push_back(loaded_job);
      }
    }
  }

  // sort all the jobs oldest highest - newest lowest

  for (auto &col : board.Columns()) {
    std::sort(col.Jobs().begin(), col.Jobs().end(), [](const kan::Job &a, const kan::Job &b) { return a.created.time < b.created.time; });
  }
}

void Board::saveJob(uint col, uint row) {
  auto job = board.Column(col).Job(row);

  YAML::Emitter out;
  out << job.toYaml();

  std::ofstream fout(_path / "jobs" / std::format("{}.yaml", job.uuid));
  fout << out.c_str();
}

void Board::saveJob(kan::Job job) {

  YAML::Emitter out;
  out << job.toYaml();

  std::ofstream fout(_path / "jobs" / std::format("{}.yaml", job.uuid));
  fout << out.c_str();
}

void Board::reloadJobs() {
  for (auto &col : board.Columns()) {
    col.Jobs().clear();
  }

  loadJobs();

}

void Board::saveIssue(Issue issue) {

  YAML::Emitter out;
  out << issue.toYaml();

  std::ofstream fout(_path / "issues" / std::format("{}.yaml", issue.uuid.to_string()));
  fout << out.c_str();
}

void Board::loadIssues() {
  for (const auto &entry : std::filesystem::directory_iterator(_path / "issues")) {

    YAML::Node config = YAML::LoadFile(entry.path());
    kan::Issue loaded_issue(config);

    board.issues.addIssue(loaded_issue);
  }

}

std::string styleToString(termui::Style style) {

  std::string modifier_codes;

  if (style.InheritsSGR()) {
    // Inherit: no action taken
  }
  else if (style.ResetsSGR()) {
    modifier_codes += "0;";
  }
  else {
    modifier_codes += "0;";

    for (const auto &attr : style.attributes) {
      modifier_codes += std::format("{};", static_cast<int>(attr));
    }
  }

  if (style.foregroundColor.IsReset()) {
    modifier_codes += "39;";
  }
  else if (!style.foregroundColor.IsInherit()) {
    if (style.foregroundColor.mode == termui::Color::Mode::Ansi256) {
      modifier_codes += std::format("38;5;{};", style.foregroundColor.value);
    }
    else if (style.foregroundColor.mode == termui::Color::Mode::TrueColor) {
      auto rgb = style.foregroundColor.rgb;
      modifier_codes += std::format("38;2;{};{};{};", rgb.r, rgb.g, rgb.b);
    }
  }

  if (style.backgroundColor.IsReset()) {
    modifier_codes += "49;";
  }
  else if (!style.backgroundColor.IsInherit()) {
    if (style.backgroundColor.mode == termui::Color::Mode::Ansi256) {
      modifier_codes += std::format("48;5;{};", style.backgroundColor.value);
    }
    else if (style.backgroundColor.mode == termui::Color::Mode::TrueColor) {
      auto rgb = style.backgroundColor.rgb;
      modifier_codes += std::format("48;2;{};{};{};", rgb.r, rgb.g, rgb.b);
    }
  }

  return modifier_codes;
}

namespace {
constexpr bool IsForegroundCode(int c) { return c == 38 || c == 39; }
constexpr bool IsBackgroundCode(int c) { return c == 48 || c == 49; }
} // namespace

termui::Style styleFromString(std::string_view s) {
  // Tokenize on ';', skipping empty fields.
  std::vector<int> codes;
  for (size_t start = 0; start <= s.size();) {
    size_t end = s.find(';', start);
    if (end == std::string_view::npos)
      end = s.size();
    if (end > start) {
      int  value = 0;
      auto res   = std::from_chars(s.data() + start, s.data() + end, value);
      if (res.ec == std::errc{})
        codes.push_back(value);
    }
    start = end + 1;
  }

  if (codes.empty()) {
    return termui::Style();
  }

  size_t i = 0;

  // --- SGR reset / attributes ---
  std::vector<termui::SGR> attrs;
  if (codes[i] == 0) {
    ++i;
    while (i < codes.size() && !IsForegroundCode(codes[i]) && !IsBackgroundCode(codes[i])) {
      attrs.push_back(static_cast<termui::SGR>(codes[i]));
      ++i;
    }
  }

  // --- Foreground ---
  termui::Color foreground;
  if (i < codes.size()) {
    if (codes[i] == 39) {
      // skip
      ++i;
    }
    else if (codes[i] == 38 && i + 1 < codes.size()) {
      if (codes[i + 1] == 5 && i + 2 < codes.size()) {
        foreground = termui::Color(codes[i + 2]);
        i += 3;
      }
      else if (codes[i + 1] == 2 && i + 4 < codes.size()) {
        foreground = termui::Color(static_cast<uint8_t>(codes[i + 2]), static_cast<uint8_t>(codes[i + 3]), static_cast<uint8_t>(codes[i + 4]));
        i += 5;
      }
    }
  }

  // --- Background ---
  termui::Color background;
  if (i < codes.size()) {
    if (codes[i] == 49) {
      // skip
      ++i;
    }
    else if (codes[i] == 48 && i + 1 < codes.size()) {
      if (codes[i + 1] == 5 && i + 2 < codes.size()) {
        background = termui::Color(codes[i + 2]);
        i += 3;
      }
      else if (codes[i + 1] == 2 && i + 4 < codes.size()) {
        background = termui::Color(static_cast<uint8_t>(codes[i + 2]), static_cast<uint8_t>(codes[i + 3]), static_cast<uint8_t>(codes[i + 4]));
        i += 5;
      }
    }
  }

  return termui::Style(foreground, background, attrs);
}

void Board::loadStyleMap() {

  YAML::Node style;

  try {
    style = YAML::LoadFile(_path / "style.yaml");
  }
  catch (const std::exception &e) {
    logMessage(std::string("Failed to load style.yaml: ") + e.what());
    return;
  
  }

  for (auto const &s : style) {
    styleMap.add(s.first.as<std::string>(), styleFromString(s.second.as<std::string>()));
  }
}

void Board::saveStyleMap() {
  auto styles = styleMap.keys();

  YAML::Node yaml;

  for (auto const &s : styles) {
    yaml[s] = styleToString(styleMap.at(s));
  }

  YAML::Emitter out;
  out << yaml;

  std::ofstream fout(_path / "style.yaml");
  fout << out.c_str();
}

std::vector<User> Board::Users() {
    std::vector<User> users;

    // Iterate through all jobs to collect users
    for (auto &column : columns) {
      const auto &jobs = column.Jobs();
      for (const auto &job : jobs) {
        // Collect creator, modifier, and assignee names
        if (!job.creator.name.empty()) {
          users.push_back(job.creator);
        }
        if (!job.modifier.name.empty()) {
          users.push_back(job.modifier);
        }
        if (!job.assignee.name.empty()) {
          users.push_back(job.assignee);
        }
      }
    }

    // Remove duplicates
    users.erase(std::unique(users.begin(), users.end()), users.end());
    
    return users;
}

// contributors.yaml holds a list of users: [{name, email}, ...]
void Board::loadContributors() {
  contributors.clear();

  auto file = _path / "contributors.yaml";
  if (!std::filesystem::exists(file)) {
    return;
  }

  for (const auto &user : YAML::LoadFile(file)) {
    contributors.push_back(User().fromYaml(user));
  }
}

bool Board::isContributor(const User &user) {
  return std::find(contributors.begin(), contributors.end(), user) != contributors.end();
}

void Board::addContributor(User user) {
  contributors.push_back(user);
  saveContributors();
}

void Board::saveContributors() {
  YAML::Node yaml(YAML::NodeType::Sequence);

  for (auto &user : contributors) {
    yaml.push_back(user.toYaml());
  }

  YAML::Emitter out;
  out << yaml;

  std::ofstream fout(_path / "contributors.yaml");
  fout << out.c_str();
}

} // namespace kan
