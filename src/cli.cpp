#include <kan/cli.hpp>
#include <kan/kan.hpp>

#include <algorithm>
#include <charconv>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <unistd.h>

namespace kan {
namespace cli {

namespace {

const char *usage_text = "kan - a kanban board that lives in your git repo\n"
                         "\n"
                         "usage:\n"
                         "  kan [path]                     open the board in <path>/.kan (default: current directory),\n"
                         "                                 creating a new board there if none exists\n"
                         "\n"
                         "  kan new <title> [options]      add a job, prints its number\n"
                         "      -d, --description <text>\n"
                         "      -p, --priority <low|normal|high>\n"
                         "      -s, --status <column>      default: the first column\n"
                         "      -a, --assignee <who>\n"
                         "  kan ls [words] [options]       list jobs on the board, words search titles, descriptions and #numbers\n"
                         "      -s, --status <column>\n"
                         "      -a, --assignee <who|none>\n"
                         "      -p, --priority <low|normal|high>\n"
                         "      --finished, --archived     list those instead, --all for everything\n"
                         "  kan show <job>                 everything about a job\n"
                         "  kan mv <job> <column>          move a job, eg kan mv 12 done\n"
                         "  kan assign <job> <who|none>    set who's working on it\n"
                         "  kan comment <job> <text>       add a comment\n"
                         "  kan finish <job>               strike it off, it moves to the finished log\n"
                         "  kan reopen <job>               back to the column it was finished from\n"
                         "  kan archive <job>              take it off the board, kan restore brings it back\n"
                         "  kan restore <job>              out of the archive, back to its old column\n"
                         "  kan branch <job>               switch to the job's branch, creating it if needed\n"
                         "  kan link-commit <hash>         link a commit to every job its message references with #N,\n"
                         "                                 and finish the jobs it closes (\"fixes #3\", \"closes #4, #5\").\n"
                         "                                 run by the post-commit hook\n"
                         "  kan help                       show this guide\n"
                         "\n"
                         "<job> is a job number, 12 or #12. <who> is me, or part of a contributor's name or email.\n"
                         "columns match without case and a unique start is enough: kan mv 12 dev\n"
                         "kan ls prints tab separated columns, without a header, when it isn't writing to a terminal.\n"
                         "to open a board in a folder named like a command, give a path: kan ./ls\n";

const std::vector<std::string> commands = {"new",     "ls",      "show",   "mv",          "assign", "comment", "finish", "reopen",
                                           "archive", "restore", "branch", "link-commit", "help",   "--help",  "-h"};

// bad arguments - exit status 2, and a hint to look at kan help
struct UsageError : std::runtime_error {
  using std::runtime_error::runtime_error;
};

std::string lower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
  return s;
}

std::string join(const std::vector<std::string> &words, size_t from = 0) {
  std::string out;
  for (size_t i = from; i < words.size(); i++) {
    out += (out.empty() ? "" : " ") + words[i];
  }
  return out;
}

// styling only when a person is reading
bool styled() {
  static bool tty = isatty(STDOUT_FILENO) && std::getenv("NO_COLOR") == nullptr;
  return tty;
}

std::string bold(const std::string &s) { return styled() ? "\x1b[1m" + s + "\x1b[22m" : s; }
std::string faint(const std::string &s) { return styled() ? "\x1b[2m" + s + "\x1b[22m" : s; }

// pads to a width in terminal columns, not bytes, so ⎇ and accents line up
std::string pad(const std::string &s, size_t width) {
  size_t visible = termui::visible_length(s);
  return visible >= width ? s : s + std::string(width - visible, ' ');
}

struct Args {
  std::vector<std::string>           words;   // positional
  std::map<std::string, std::string> options; // long name -> value
  std::set<std::string>              flags;
};

// options can go anywhere, "--" ends them. takes = options that need a value, keyed by long name, with their short letter
Args parse(const std::vector<std::string> &args, const std::map<std::string, char> &takes, const std::set<std::string> &flags = {}) {
  Args out;

  for (size_t i = 1; i < args.size(); i++) {
    auto arg = args[i];

    if (arg == "--") {
      out.words.insert(out.words.end(), args.begin() + i + 1, args.end());
      break;
    }

    if (arg.size() < 2 || arg[0] != '-' || std::isdigit(static_cast<unsigned char>(arg[1]))) {
      out.words.push_back(arg);
      continue;
    }

    std::string name, value;
    bool        has_value = false;

    if (arg.starts_with("--")) {
      name = arg.substr(2);

      if (auto eq = name.find('='); eq != std::string::npos) {
        value     = name.substr(eq + 1);
        name      = name.substr(0, eq);
        has_value = true;
      }
    }
    else {
      auto it = std::find_if(takes.begin(), takes.end(), [&](const auto &t) { return arg.size() == 2 && t.second == arg[1]; });
      name    = it == takes.end() ? arg : it->first;
    }

    if (flags.contains(name)) {
      out.flags.insert(name);
      continue;
    }

    if (!takes.contains(name)) {
      throw UsageError(std::format("unknown option {}", arg));
    }

    if (!has_value) {
      if (i + 1 >= args.size()) {
        throw UsageError(std::format("{} needs a value", arg));
      }
      value = args[++i];
    }

    out.options[name] = value;
  }

  return out;
}

std::filesystem::path findBoard() {
  for (auto dir = std::filesystem::current_path();; dir = dir.parent_path()) {
    if (std::filesystem::is_directory(dir / ".kan")) {
      return dir / ".kan";
    }

    if (dir == dir.parent_path()) {
      throw std::runtime_error("no board here or in any parent folder - run kan to make one");
    }
  }
}

void openBoard() {
  board = Board(findBoard());
  board.loadBoard();
  board.loadJobs();
  board.loadContributors();
}

int jobId(const std::string &ref) {
  auto digits = ref.starts_with("#") ? ref.substr(1) : ref;

  int id         = 0;
  auto [ptr, ec] = std::from_chars(digits.data(), digits.data() + digits.size(), id);

  if (ec != std::errc() || ptr != digits.data() + digits.size() || id <= 0) {
    throw UsageError(std::format("{} isn't a job number", ref));
  }

  return id;
}

kan::Job findJob(const std::string &ref) {
  auto id  = jobId(ref);
  auto job = board.findJob(id);

  if (job == nullptr) {
    throw std::runtime_error(std::format("there's no job #{}", id));
  }

  return *job;
}

kan::Job::Priority priority(const std::string &name) {
  auto p = lower(name);

  if (p == "low") {
    return kan::Job::Priority::Low;
  }
  if (p == "normal") {
    return kan::Job::Priority::Normal;
  }
  if (p == "high") {
    return kan::Job::Priority::High;
  }

  throw UsageError(std::format("priority is low, normal or high, not {}", name));
}

std::string priorityName(kan::Job::Priority p) { return YAML::Node(p).as<std::string>(); }

std::string column(const std::string &name) {
  auto statuses = board.Statuses();
  return statuses.at(matchColumn(statuses, name));
}

// me, none, or a contributor (or anyone on a job) by name or email - exact first, then a unique part
User user(const std::string &who) {
  if (lower(who) == "me") {
    auto me = User().getFromSystem();
    if (me.name.empty()) {
      throw std::runtime_error("git doesn't know who you are, set user.name and user.email");
    }
    return me;
  }

  if (lower(who) == "none" || lower(who) == "nobody") {
    return User();
  }

  std::vector<User> people = board.contributors;
  for (auto &u : board.Users()) {
    if (std::find(people.begin(), people.end(), u) == people.end()) {
      people.push_back(u);
    }
  }

  auto w = lower(who);

  for (auto &u : people) {
    if (lower(u.name) == w || lower(u.email) == w) {
      return u;
    }
  }

  std::vector<User> partial;
  for (auto &u : people) {
    if (lower(u.name).find(w) != std::string::npos || lower(u.email).find(w) != std::string::npos) {
      partial.push_back(u);
    }
  }

  if (partial.size() == 1) {
    return partial.front();
  }

  if (partial.empty()) {
    throw std::runtime_error(std::format("nobody called {} is on this board", who));
  }

  std::string names;
  for (auto &u : partial) {
    names += "\n  " + u.toString();
  }
  throw std::runtime_error(std::format("{} could be any of:{}", who, names));
}

std::string assigneeName(kan::Job &job) { return job.assignee.name.empty() ? std::string("-") : job.assignee.Name(); }

std::string where(kan::Job &job) { return job.archived ? std::string("archived") : job.finished ? std::string("finished") : job.status; }

void needWords(const Args &args, size_t n, const std::string &usage) {
  if (args.words.size() < n) {
    throw UsageError("usage: kan " + usage);
  }
}

// ---- commands ----

int cmdNew(const std::vector<std::string> &raw) {
  auto args = parse(raw, {{"description", 'd'}, {"priority", 'p'}, {"status", 's'}, {"assignee", 'a'}});
  needWords(args, 1, "new <title> [options]");

  openBoard();

  if (board.Statuses().empty()) {
    throw std::runtime_error("the board has no columns");
  }

  kan::Job job;
  job.title  = join(args.words);
  job.status = board.Statuses().front();

  if (args.options.contains("description")) {
    job.description = args.options["description"];
  }
  if (args.options.contains("priority")) {
    job.priority = priority(args.options["priority"]);
  }
  if (args.options.contains("status")) {
    job.status = column(args.options["status"]);
  }
  if (args.options.contains("assignee")) {
    job.assignee = user(args.options["assignee"]);
  }

  job.history.addEvent({"Job created.", User().getFromSystem(), Timestamp().Now()});
  job.id = board.nextJobId();

  board.saveJob(job);

  std::cout << std::format("{} {} {}\n", bold("#" + std::to_string(job.id)), job.title, faint("in " + job.status));
  return 0;
}

int cmdLs(const std::vector<std::string> &raw) {
  auto args = parse(raw, {{"status", 's'}, {"assignee", 'a'}, {"priority", 'p'}}, {"finished", "archived", "all"});

  openBoard();

  std::vector<kan::Job> jobs;
  bool                  all = args.flags.contains("all");

  if (all || (!args.flags.contains("finished") && !args.flags.contains("archived"))) {
    for (auto &col : board.Columns()) {
      jobs.insert(jobs.end(), col.Jobs().begin(), col.Jobs().end());
    }
  }
  if (all || args.flags.contains("finished")) {
    jobs.insert(jobs.end(), board.finished.begin(), board.finished.end());
  }
  if (all || args.flags.contains("archived")) {
    jobs.insert(jobs.end(), board.archive.begin(), board.archive.end());
  }

  auto query = join(args.words);

  if (args.options.contains("status")) {
    auto status = column(args.options["status"]);
    std::erase_if(jobs, [&](const kan::Job &job) { return job.status != status; });
  }
  if (args.options.contains("assignee")) {
    auto who = user(args.options["assignee"]);
    std::erase_if(jobs, [&](const kan::Job &job) { return !(job.assignee == who); });
  }
  if (args.options.contains("priority")) {
    auto p = priority(args.options["priority"]);
    std::erase_if(jobs, [&](const kan::Job &job) { return job.priority != p; });
  }

  std::erase_if(jobs, [&](const kan::Job &job) { return !tui::jobMatches(job, query); });

  if (!styled()) { // for scripts: id, status, priority, assignee, title
    for (auto &job : jobs) {
      std::cout << std::format("{}\t{}\t{}\t{}\t{}\n", job.id, where(job), priorityName(job.priority), job.assignee.name, job.title);
    }
    return 0;
  }

  if (jobs.empty()) {
    std::cout << faint("No jobs.") << "\n";
    return 0;
  }

  size_t id_w = 2, status_w = 6, assignee_w = 8;
  for (auto &job : jobs) {
    id_w       = std::max(id_w, std::to_string(job.id).size() + 1);
    status_w   = std::max(status_w, termui::visible_length(where(job)));
    assignee_w = std::min<size_t>(20, std::max(assignee_w, termui::visible_length(assigneeName(job))));
  }

  std::cout << bold(std::format("{}  {}  {}  {}  {}", pad("#", id_w), pad("STATUS", status_w), pad("PRIORITY", 8), pad("ASSIGNEE", assignee_w), "TITLE"))
            << "\n";

  for (auto &job : jobs) {
    auto line = std::format("{}  {}  {}  {}  {}", pad("#" + std::to_string(job.id), id_w), pad(where(job), status_w), pad(priorityName(job.priority), 8),
                            pad(assigneeName(job), assignee_w), job.title);

    std::cout << (job.finished || job.archived ? faint(line) : line) << "\n";
  }

  return 0;
}

int cmdShow(const std::vector<std::string> &raw) {
  auto args = parse(raw, {});
  needWords(args, 1, "show <job>");

  openBoard();
  board.loadBranches();

  auto job = findJob(args.words[0]);

  auto row = [](const std::string &label, const std::string &value) { std::cout << faint(pad(label, 10)) << value << "\n"; };

  std::cout << bold(job.title) << " " << faint("#" + std::to_string(job.id)) << "\n\n";

  if (job.finished) {
    row("Status", std::format("finished from {} by {} on {}", job.finished_from, job.finisher.Name(), job.finished_at.toString()));
  }
  else {
    row("Status", job.archived ? job.status + " (archived)" : job.status);
  }

  row("Priority", priorityName(job.priority));
  row("Assignee", job.assignee.name.empty() ? std::string("No assignee") : job.assignee.toString());

  if (auto branch = board.branchFor(job.id)) {
    row("Branch", branch->name + (branch->current ? " (checked out)" : branch->merged ? " (merged)" : ""));
  }

  row("Created", std::format("{} · {}", job.creator.toString(), job.created.toString()));
  row("Updated", std::format("{} · {}", job.modifier.toString(), job.updated.toString()));

  if (!job.description.empty()) {
    std::cout << "\n" << job.description << "\n";
  }

  if (job.comments.size() > 0) {
    std::cout << "\n" << bold(std::format("Comments ({})", job.comments.size())) << "\n";

    for (auto &c : job.comments) {
      std::cout << faint(std::format("  {} · {}", c.creator.Name(), c.created.toString())) << "\n";
      std::cout << "  " << std::string(c.text) << "\n";
    }
  }

  if (job.commits.size() > 0) {
    std::cout << "\n" << bold(std::format("Commits ({})", job.commits.size())) << "\n";

    for (auto &c : job.commits) {
      auto title = c.message.substr(0, c.message.find('\n'));
      std::cout << "  " << faint(c.hash.substr(0, 7)) << "  " << title << "\n";
    }
  }

  return 0;
}

int cmdMv(const std::vector<std::string> &raw) {
  auto args = parse(raw, {});
  needWords(args, 2, "mv <job> <column>");

  openBoard();

  auto job = findJob(args.words[0]);
  auto to  = column(join(args.words, 1)); // columns can have spaces: kan mv 12 in review

  if (job.archived) {
    throw std::runtime_error(std::format("#{} is archived, kan restore {} first", job.id, job.id));
  }

  if (job.finished) { // moving a finished job puts it back on the board
    job.reopen(to);
    board.saveJob(job);

    std::cout << std::format("{} reopened to {}\n", bold("#" + std::to_string(job.id)), to);
    return 0;
  }

  if (job.status == to) {
    std::cout << std::format("{} is already in {}\n", bold("#" + std::to_string(job.id)), to);
    return 0;
  }

  auto from = job.status;

  job.status = to;
  job.addEvent({std::format("Job status was updated: {} -> {}.", from, to), User().getFromSystem(), Timestamp().Now()});

  if (board.finish_on_last_column && to == board.Statuses().back()) {
    board.finishJob(job);
    std::cout << std::format("{} {} -> {}, finished\n", bold("#" + std::to_string(job.id)), from, to);
    return 0;
  }

  board.saveJob(job);

  std::cout << std::format("{} {} -> {}\n", bold("#" + std::to_string(job.id)), from, to);
  return 0;
}

int cmdAssign(const std::vector<std::string> &raw) {
  auto args = parse(raw, {});
  needWords(args, 2, "assign <job> <who|none>");

  openBoard();

  auto job = findJob(args.words[0]);
  auto who = user(join(args.words, 1));

  if (job.assignee == who) {
    std::cout << std::format("{} is already assigned to {}\n", bold("#" + std::to_string(job.id)), assigneeName(job));
    return 0;
  }

  auto old = job.assignee.name.empty() ? std::string("None") : job.assignee.Name();

  job.assignee = who;
  job.addEvent({std::format("Job assignee was updated: {} -> {}.", old, who.name.empty() ? std::string("None") : who.Name()), User().getFromSystem(),
                Timestamp().Now()});

  board.saveJob(job);

  std::cout << std::format("{} assigned to {}\n", bold("#" + std::to_string(job.id)), assigneeName(job));
  return 0;
}

int cmdComment(const std::vector<std::string> &raw) {
  auto args = parse(raw, {});
  needWords(args, 2, "comment <job> <text>");

  openBoard();

  auto job = findJob(args.words[0]);

  kan::Comment comment;
  comment.text    = join(args.words, 1);
  comment.creator = User().getFromSystem();
  comment.created = Timestamp().Now();

  job.comments.addComment(comment);
  job.addEvent({"Comment added.", comment.creator, comment.created});

  board.saveJob(job);

  std::cout << std::format("{} comment added\n", bold("#" + std::to_string(job.id)));
  return 0;
}

int cmdFinish(const std::vector<std::string> &raw) {
  auto args = parse(raw, {});
  needWords(args, 1, "finish <job>");

  openBoard();

  auto job = findJob(args.words[0]);

  if (job.archived) {
    throw std::runtime_error(std::format("#{} is archived", job.id));
  }

  if (job.finished) {
    std::cout << std::format("{} is already finished\n", bold("#" + std::to_string(job.id)));
    return 0;
  }

  board.finishJob(job);

  std::cout << std::format("{} finished\n", bold("#" + std::to_string(job.id)));
  return 0;
}

int cmdReopen(const std::vector<std::string> &raw) {
  auto args = parse(raw, {});
  needWords(args, 1, "reopen <job>");

  openBoard();

  auto job = findJob(args.words[0]);

  if (!job.finished) {
    throw std::runtime_error(std::format("#{} isn't finished", job.id));
  }

  board.reopenJob(job);

  std::cout << std::format("{} reopened to {}\n", bold("#" + std::to_string(job.id)), findJob(args.words[0]).status);
  return 0;
}

int cmdArchive(const std::vector<std::string> &raw) {
  auto args = parse(raw, {});
  needWords(args, 1, "archive <job>");

  openBoard();

  auto job = findJob(args.words[0]);

  if (job.archived) {
    std::cout << std::format("{} is already archived\n", bold("#" + std::to_string(job.id)));
    return 0;
  }

  board.archiveJob(job);

  std::cout << std::format("{} archived\n", bold("#" + std::to_string(job.id)));
  return 0;
}

int cmdRestore(const std::vector<std::string> &raw) {
  auto args = parse(raw, {});
  needWords(args, 1, "restore <job>");

  openBoard();

  auto job = findJob(args.words[0]);

  if (!job.archived) {
    throw std::runtime_error(std::format("#{} isn't archived", job.id));
  }

  board.restoreJob(job);

  std::cout << std::format("{} restored to {}\n", bold("#" + std::to_string(job.id)), findJob(args.words[0]).status);
  return 0;
}

int cmdBranch(const std::vector<std::string> &raw) {
  auto args = parse(raw, {});
  needWords(args, 1, "branch <job>");

  openBoard();
  board.loadBranches();

  auto job    = findJob(args.words[0]);
  auto branch = board.branchFor(job.id);

  if (branch && branch->current) {
    std::cout << std::format("already on {}\n", branch->name);
    return 0;
  }

  auto name   = branch ? branch->name : git::jobBranchName(job.id, job.title);
  auto create = branch == nullptr;

  if (auto error = git::switchBranch(name, create); !error.empty()) {
    throw std::runtime_error(error);
  }

  if (create) {
    job.addEvent({std::format("Branch {} created.", name), User().getFromSystem(), Timestamp().Now()});
    board.saveJob(job);
  }

  std::cout << std::format("switched to {}{}\n", create ? "a new branch " : "", name);
  return 0;
}

// called by the post-commit hook
int cmdLinkCommit(const std::vector<std::string> &raw) {
  auto args = parse(raw, {});
  needWords(args, 1, "link-commit <hash>");

  openBoard();
  hooks::postCommit(board, args.words[0]);

  return 0;
}

} // namespace

int matchColumn(const std::vector<std::string> &columns, const std::string &name) {
  auto n = lower(name);

  for (size_t i = 0; i < columns.size(); i++) {
    if (lower(columns[i]) == n) {
      return i;
    }
  }

  std::vector<size_t> starts;
  for (size_t i = 0; i < columns.size(); i++) {
    if (!n.empty() && lower(columns[i]).starts_with(n)) {
      starts.push_back(i);
    }
  }

  if (starts.size() == 1) {
    return starts.front();
  }

  std::vector<std::string> options;
  for (size_t i = 0; i < columns.size(); i++) {
    if (starts.empty() || std::find(starts.begin(), starts.end(), i) != starts.end()) {
      options.push_back(columns[i]);
    }
  }

  std::string list;
  for (auto &o : options) {
    list += (list.empty() ? "" : ", ") + o;
  }

  throw std::runtime_error(starts.empty() ? std::format("there's no column called {} - the columns are {}", name, list)
                                          : std::format("{} could be {}", name, list));
}

bool isCommand(const std::string &arg) { return std::find(commands.begin(), commands.end(), arg) != commands.end(); }

int run(const std::vector<std::string> &args) {
  static const std::map<std::string, int (*)(const std::vector<std::string> &)> handlers = {
      {"new", cmdNew},       {"ls", cmdLs},         {"show", cmdShow},       {"mv", cmdMv},           {"assign", cmdAssign}, {"comment", cmdComment},
      {"finish", cmdFinish}, {"reopen", cmdReopen}, {"archive", cmdArchive}, {"restore", cmdRestore}, {"branch", cmdBranch}, {"link-commit", cmdLinkCommit},
  };

  auto it = args.empty() ? handlers.end() : handlers.find(args[0]);

  if (it == handlers.end()) {
    std::cout << usage_text;
    return 0;
  }

  try {
    return it->second(args);
  }
  catch (const UsageError &e) {
    std::cerr << "kan: " << e.what() << "\n(kan help lists every command)\n";
    return 2;
  }
  catch (const std::exception &e) {
    std::cerr << "kan " << args[0] << ": " << e.what() << "\n";
    return 1;
  }
}

} // namespace cli
} // namespace kan
