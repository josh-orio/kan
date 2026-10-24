#include <filesystem>
#include <format>
#include <fstream>
#include <kan/core.hpp>

namespace kan {

Board &Board::instance() {
  static Board board = Board("");
  return board;
}

Board::Board(std::string path) : _path(path) {}

// Board::Board(nlohmann::json board) : _title(board["title"]) {
//   for (auto col : board["columns"]) {
//     _columns.push_back(Column(col));
//   }
// }

// nlohmann::json Board::toJson() {
//   nlohmann::json board;

//   board["title"] = _title;

//   std::vector<nlohmann::json> col_jsons;
//   for (auto &c : _columns) {
//     col_jsons.push_back(c.toJson());
//   }

//   board["columns"] = col_jsons;

//   return board;
// }

void Board::fromYaml(YAML::Node yaml) {
  title = yaml["title"].as<std::string>();

  for (const auto &col : yaml["columns"]) {
    columns.push_back(kan::Column(col));
  }
}

YAML::Node Board::toYaml() {
  YAML::Node yaml;
  yaml["title"] = title;

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

// void Board::fromJson(nlohmann::json json) {
//   _title = json["title"];

//   kan::Column column;
//   for (auto col : json["columns"]) {
//     column.fromJson(col);
//     _columns.push_back(column);
//   }
// };

std::string Board::Title() { return title; }

std::vector<Column> &Board::Columns() { return columns; }

Column &Board::Column(unsigned int idx) { return columns.at(idx); }

std::vector<Job> &Board::Jobs(unsigned int col_idx) { return columns.at(col_idx).Jobs(); }

Job &Board::Job(unsigned int col_idx, unsigned int row_idx) { return columns.at(col_idx).Job(row_idx); }

void Board::deleteJob(kan::Job job) {

  for (auto &col : board.Columns()) {
    auto &jobs = col.Jobs();
    jobs.erase(std::remove_if(jobs.begin(), jobs.end(), [&](const kan::Job &j) { return j.uuid == job.uuid; }), jobs.end());
  }

  // for (auto &col : _columns) {
  //   for (auto & jjob :col.Jobs()) {
  //     if (job.uuid == jjob.uuid) {

  //     }
  //   }
  // }
}

// void Board::setTitle(std::string s) {_title=s;}
// // void Board::setDescription(std::string s){}
// void Board::addColumn(std::string title) {
//   Column c;
//   c.
// }

void Board::loadBoard() {

  //  std::ifstream  in_file(_path / "kan.bson", std::ios::binary);
  // nlohmann::json j_loaded = nlohmann::json::from_bson(in_file);

  // std::ifstream  in_file(_path / "kan.json");
  // nlohmann::json j_loaded = nlohmann::json::parse(in_file);

  // board.fromJson(j_loaded);

  YAML::Node config = YAML::LoadFile(_path / "kan.yaml");
  board.fromYaml(config);
}

void Board::saveBoard() {
  // auto json = board.toJson();
  // auto bson = nlohmann::json::to_bson(json);

  // std::ofstream out_file(_path / "kan.bson", std::ios::binary);
  // out_file.write(reinterpret_cast<const char *>(bson.data()), bson.size());
  // out_file.close();
}

void Board::loadJobs() {
  // kan::Job loaded_job;

  for (const auto &entry : std::filesystem::directory_iterator(_path / "jobs")) {

    // std::cout << entry.path() << std::endl;

    // try {
    // std::ifstream in_file = std::ifstream(entry.path());

    // loaded_job.fromJson(nlohmann::json::parse(in_file));

    YAML::Node config = YAML::LoadFile(entry.path());
    // board.fromYaml(config);
    kan::Job loaded_job(config);

    for (auto &col : board.Columns()) {
      if (col.title == loaded_job.status) {
        // col._jobs.push_back(loaded_job);
        // board.
        col.Jobs().push_back(loaded_job);
      }
    }

    // } catch (std::exception ex) {
    //   std::cout << ex.what() << std::endl;
    // }
  }

  // sort all the jobs oldest highest - newest lowest

  for (auto &col : board.Columns()) {
    std::sort(col.Jobs().begin(), col.Jobs().end(), [](const kan::Job &a, const kan::Job &b) { return a.created.time < b.created.time; });
  }
}

// void KAN::saveJobs() {
//   // save all board files - probably not needed
// }

// void KAN::loadJob() {
//   // how would you load the file without knowing its name
//   // why would you even load an individual job?
// }

void Board::saveJob(uint col, uint row) {
  auto job = board.Column(col).Job(row);

  // std::ofstream out_file_job(_path / "jobs" / std::format("{}.bson", first_job.uuid), std::ios::binary);
  // nlohmann::json::to_bson(first_job.toJson(), out_file_job);
  // std::ofstream out_file_job(_path / "jobs" / std::format("{}.json", job.uuid));
  // out_file_job << job.toJson().dump(2);

  YAML::Emitter out;
  out << job.toYaml();
  // out << YAML::BeginMap;
  // out << YAML::Key << "name" << YAML::Value << "Fix bug";
  // out << YAML::Key << "priority" << YAML::Value << "High";
  // out << YAML::EndMap;

  std::ofstream fout(_path / "jobs" / std::format("{}.yaml", job.uuid));
  fout << out.c_str();
}

void Board::saveJob(kan::Job job) {
  // auto job = _board._columns.at(col)._jobs.at(row);

  // std::ofstream out_file_job(_path / "jobs" / std::format("{}.bson", first_job.uuid), std::ios::binary);
  // nlohmann::json::to_bson(first_job.toJson(), out_file_job);
  // std::ofstream out_file_job(_path / "jobs" / std::format("{}.json", job.uuid));
  // out_file_job << job.toJson().dump(2);

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
  // for (auto col : board.Columns()) {
  //   for (auto job : col.Jobs()) {
  //     if (job.status != col.Title()) {
  //       for (auto col2 : board.Columns()) {
  //         if (col2.Title() == job.status) {
  //           col2.Jobs().push_back(job);
  //           // col.Jobs().erase(job);
  //         }
  //       }
  //     }
  //   }
  // }

  // for (auto &col : board.Columns()) {
  //   auto &jobs = col.Jobs();
  //   jobs.erase(std::remove_if(jobs.begin(), jobs.end(), [&](const Job &j) { return j.status != col.Title(); }), jobs.end());
  // }
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

  // // sort all the jobs oldest highest - newest lowest

  // for (auto &col : board.Columns()) {
  //   std::sort(col.Jobs().begin(), col.Jobs().end(), [](const kan::Job &a, const kan::Job &b) { return a.created.time < b.created.time; });
  // }
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

// termui::Style styleFromString(std::string style) {
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
  // YAML::Node style = YAML::LoadFile(_path / "style.yaml");
  // YAML::Node style;

  // try {
  //   style = YAML::LoadFile(_path / "style.yaml");
  // }
  // // catch (const YAML::BadFile &) {
  // catch (...) {
  // }

  // // std::cout <<1234<<"\n";
  // // std::exit(1234);

  // if (!style || !style.IsDefined()) {
  //   logMessage("Failed to load the stylemap - style.yaml may be missing or malformed.");
  //   return;
  // }
  // std::cout <<1234<<"\n";
  // std::exit(1234);

  YAML::Node style;
  // bool       loaded = false;

  try {
    style = YAML::LoadFile(_path / "style.yaml");
    // loaded = true;
  }
  catch (const std::exception &e) {
    logMessage(std::string("Failed to load style.yaml: ") + e.what());
    return;
  
  }

  // if (!loaded) {
  //   logMessage("Failed to load the stylemap - style.yaml may be missing or malformed.");
  //   return;
  // }

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
    // std::sort(users.begin(), users.end());
    users.erase(std::unique(users.begin(), users.end()), users.end());
    
    return users;
}

} // namespace kan
