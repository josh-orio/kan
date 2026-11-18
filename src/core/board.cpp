#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <kan/core.hpp>
#include <set>

namespace kan {

Board &Board::instance() {
  static Board board = Board("");
  return board;
}

Board::Board(std::string path) : _path(path) {}

std::filesystem::path Board::Path() { return _path; }

void Board::fromYaml(YAML::Node yaml) {
  title   = yaml["title"].as<std::string>();
  next_id = yaml["next_id"].as<int>(1);

  finish_on_last_column = yaml["finish_on_last_column"].as<bool>(false);

  for (const auto &col : yaml["columns"]) {
    columns.push_back(kan::Column(col));
  }
}

YAML::Node Board::toYaml() {
  YAML::Node yaml;
  yaml["title"]   = title;
  yaml["next_id"] = next_id;

  yaml["finish_on_last_column"] = finish_on_last_column;

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

  for (auto &job : finished) {
    if (job.id == id) {
      return &job;
    }
  }

  for (auto &job : archive) { // commits can still be linked to archived jobs
    if (job.id == id) {
      return &job;
    }
  }

  return nullptr;
}

std::string Board::columnNameProblem(std::string name, int skip) {
  if (name.find_first_not_of(" ") == std::string::npos) {
    return "A column needs a name.";
  }

  auto lower = [](std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
    return s;
  };

  for (int i = 0; i < static_cast<int>(columns.size()); i++) {
    if (i != skip && lower(columns[i].title) == lower(name)) {
      return std::format("There's already a column called {}.", columns[i].title);
    }
  }

  return "";
}

void Board::addColumn(std::string title, size_t at) {
  kan::Column column;
  column.title = title;

  columns.insert(columns.begin() + std::min(at, columns.size()), column);

  saveBoard();
  reloadJobs();
}

// every job that points at a column by name, wherever it lives - on the board, finished or archived
static void forEachJob(Board &b, const std::function<void(kan::Job &)> &fn) {
  for (auto &col : b.columns) {
    for (auto &job : col.Jobs()) {
      fn(job);
    }
  }
  for (auto &job : b.finished) {
    fn(job);
  }
  for (auto &job : b.archive) {
    fn(job);
  }
}

void Board::renameColumn(size_t idx, std::string title) {
  auto old = columns.at(idx).title;

  // the job files are rewritten, the history keeps the old name as it was at the time
  forEachJob(*this, [&](kan::Job &job) {
    bool changed = false;

    if (job.status == old) {
      job.status = title;
      changed    = true;
    }
    if (job.finished_from == old) {
      job.finished_from = title;
      changed           = true;
    }

    if (changed) {
      saveJob(job);
    }
  });

  columns.at(idx).title = title;

  saveBoard();
  reloadJobs();
}

void Board::removeColumn(size_t idx, size_t move_jobs_to) {
  auto old = columns.at(idx).title;
  auto to  = columns.at(move_jobs_to).title;

  for (auto &job : columns.at(idx).Jobs()) {
    if (job.finished) { // struck through, saved with the finished ones below
      continue;
    }

    job.status = to;
    job.addEvent({std::format("Job status was updated: {} -> {}. The {} column was removed.", old, to, old), User().getFromSystem(), Timestamp().Now()});
    saveJob(job);
  }

  // finished and archived jobs come back to the new column too
  for (auto *list : {&finished, &archive}) {
    for (auto &job : *list) {
      bool changed = false;

      if (job.status == old) {
        job.status = to;
        changed    = true;
      }
      if (job.finished_from == old) {
        job.finished_from = to;
        changed           = true;
      }

      if (changed) {
        saveJob(job);
      }
    }
  }

  columns.erase(columns.begin() + idx);

  saveBoard();
  reloadJobs();
}

void Board::moveColumn(size_t from, size_t to) {
  if (from >= columns.size() || to >= columns.size() || from == to) {
    return;
  }

  auto column = columns.at(from);
  columns.erase(columns.begin() + from);
  columns.insert(columns.begin() + to, column);

  saveBoard();
}

void Board::setColumnHeader(size_t idx, termui::Style style) {
  columns.at(idx).header = style;
  saveBoard();
}

int Board::nextJobId() {
  int id = next_id++;
  saveBoard();

  return id;
}

void Board::deleteJob(kan::Job job) {
  record(job.uuid, "");
  std::filesystem::remove(_path / "jobs" / std::format("{}.yaml", job.uuid));

  reloadJobs();
}

void Board::archiveJob(kan::Job job) {
  job.archived = true;
  job.addEvent({"Job archived.", User().getFromSystem(), Timestamp().Now()});

  saveJob(job);
  reloadJobs();
}

void Board::finishJob(kan::Job job) {
  job.finish(User().getFromSystem(), Timestamp().Now());

  saveJob(job); // before striking it, so undo remembers it wasn't
  struck.insert(job.uuid);
  reloadJobs();
}

void Board::reopenJob(kan::Job job) {
  auto statuses = Statuses();
  auto to       = job.finished_from.empty() ? job.status : job.finished_from;

  if (std::find(statuses.begin(), statuses.end(), to) == statuses.end() && !statuses.empty()) {
    to = statuses.front();
  }

  job.reopen(to);

  saveJob(job);
  struck.erase(job.uuid);
  reloadJobs();
}

void Board::restoreJob(kan::Job job) {
  auto statuses = Statuses();

  if (std::find(statuses.begin(), statuses.end(), job.status) == statuses.end() && !statuses.empty()) {
    job.status = statuses.front();
  }

  job.archived = false;
  job.addEvent({std::format("Job restored from the archive to {}.", job.status), User().getFromSystem(), Timestamp().Now()});

  saveJob(job);
  reloadJobs();
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
    if (loaded_job.archived) {
      archive.push_back(loaded_job);
      continue;
    }

    if (loaded_job.finished) {
      finished.push_back(loaded_job);

      if (!struck.contains(loaded_job.uuid)) { // struck jobs also stay in their column for now
        continue;
      }
    }

    auto col = std::find_if(board.Columns().begin(), board.Columns().end(), [&](kan::Column &c) { return c.title == loaded_job.status; });

    // its column has gone, eg renamed on another branch - show it in the first column rather than lose it
    if (col == board.Columns().end() && !board.Columns().empty()) {
      col = board.Columns().begin();
    }

    if (col != board.Columns().end()) {
      col->Jobs().push_back(loaded_job);
    }
  }

  // sort all the jobs oldest highest - newest lowest

  for (auto &col : board.Columns()) {
    std::sort(col.Jobs().begin(), col.Jobs().end(), [](const kan::Job &a, const kan::Job &b) { return a.created.time < b.created.time; });
  }

  // archiving is the last thing that happens to a job, so updated is when it was archived
  std::sort(archive.begin(), archive.end(), [](const kan::Job &a, const kan::Job &b) { return a.updated.time > b.updated.time; });
  std::sort(finished.begin(), finished.end(), [](const kan::Job &a, const kan::Job &b) { return a.finished_at.time > b.finished_at.time; });
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

  record(job.uuid, out.c_str());

  std::ofstream fout(_path / "jobs" / std::format("{}.yaml", job.uuid));
  fout << out.c_str();
}

std::string Board::readJobFile(const std::string &uuid) {
  std::ifstream in(_path / "jobs" / std::format("{}.yaml", uuid));
  return std::string(std::istreambuf_iterator<char>(in), {});
}

void Board::record(const std::string &uuid, const std::string &after) {
  if (!_recording) {
    return;
  }

  auto &files = _undo.back().files;

  if (!files.contains(uuid)) {
    files[uuid] = {readJobFile(uuid), "", struck.contains(uuid)};
  }

  files[uuid].after = after;
}

void Board::beginUndo(std::string label) {
  endUndo();

  _undo.push_back({label, {}});
  _recording = true;

  if (_undo.size() > 100) {
    _undo.erase(_undo.begin());
  }
}

void Board::endUndo() {
  if (_recording && _undo.back().files.empty()) {
    _undo.pop_back();
  }

  _recording = false;
}

bool Board::canUndo() { return !_undo.empty() && !_recording; }

std::string Board::undo() {
  if (!canUndo()) {
    return "Nothing to undo.";
  }

  auto step = _undo.back();
  _undo.pop_back();

  // someone else (a git pull, the cli, a commit) changed one of these since - putting it back would lose their change
  for (auto &[uuid, file] : step.files) {
    if (readJobFile(uuid) != file.after) {
      return std::format("Can't undo {} - the job has been changed since.", step.label);
    }
  }

  for (auto &[uuid, file] : step.files) {
    auto path = _path / "jobs" / std::format("{}.yaml", uuid);

    if (file.before.empty()) {
      std::filesystem::remove(path);
    }
    else {
      std::ofstream(path) << file.before;
    }

    file.struck ? (void)struck.insert(uuid) : (void)struck.erase(uuid);
  }

  reloadJobs();

  return std::format("Undid {}.", step.label);
}

void Board::reloadJobs() {
  for (auto &col : board.Columns()) {
    col.Jobs().clear();
  }
  archive.clear();
  finished.clear();

  loadJobs();
}

bool Board::refresh() {
  kan::Board fresh(_path);
  fresh.fromYaml(YAML::LoadFile(_path / "kan.yaml"));

  title                 = fresh.title;
  next_id               = std::max(next_id, fresh.next_id);
  finish_on_last_column = fresh.finish_on_last_column;

  bool replace_columns = fresh.Statuses() != Statuses();
  if (replace_columns) {
    columns = fresh.columns;
  }
  else {
    for (size_t i = 0; i < columns.size(); i++) {
      columns[i].header = fresh.columns[i].header;
    }
  }

  // a job finished while it's on the board, by a commit say, gets struck through like one finished here
  std::set<std::string> on_board;
  for (auto &col : columns) {
    for (auto &job : col.Jobs()) {
      if (!job.finished) {
        on_board.insert(job.uuid);
      }
    }
  }

  reloadJobs();

  bool newly_struck = false;
  for (auto &job : finished) {
    if (on_board.contains(job.uuid)) {
      newly_struck |= struck.insert(job.uuid).second;
    }
  }

  if (newly_struck) {
    reloadJobs();
  }

  loadContributors();

  return replace_columns;
}

void Board::loadBranches() {
  branches.clear();

  // the checked out branch wins if a job has a few, then one still being worked on
  auto rank = [](const git::Branch &b) { return b.current ? 2 : !b.merged ? 1 : 0; };

  for (auto &branch : git::branches()) {
    int id = git::branchJobId(branch.name);

    if (id == 0) {
      continue;
    }

    if (auto it = branches.find(id); it == branches.end() || rank(branch) > rank(it->second)) {
      branches[id] = branch;
    }
  }
}

const git::Branch *Board::branchFor(int id) {
  auto it = branches.find(id);
  return it == branches.end() ? nullptr : &it->second;
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

bool Board::isContributor(const User &user) { return std::find(contributors.begin(), contributors.end(), user) != contributors.end(); }

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
