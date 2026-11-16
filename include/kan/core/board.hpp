#ifndef CORE_BOARD_HPP
#define CORE_BOARD_HPP

#include <filesystem>
#include <kan/core/column.hpp>
#include <kan/core/git.hpp>
#include <map>
#include <set>
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
  std::vector<kan::Job>    archive;      // archived jobs, most recently archived first
  std::vector<kan::Job>    finished;     // finished jobs, most recently finished first
  std::set<std::string>    struck;       // uuids finished this session - they stay on the board, struck through, until kan closes
  bool                     finish_on_last_column = false; // moving a job into the last column finishes it
  std::map<int, git::Branch> branches;                    // job id -> its branch (12-fix-login-crash), read from git

  termui::StyleMap styleMap;

  static Board &instance();

  Board(std::string path);

  std::filesystem::path Path(); // the .kan directory

  void       fromYaml(YAML::Node yaml);
  YAML::Node toYaml();

  std::vector<std::string> Statuses();
  std::vector<User>        Users();

  std::string          Title();
  std::vector<Column> &Columns();

  Column           &Column(unsigned int idx);
  std::vector<Job> &Jobs(unsigned int col_idx);
  Job              &Job(unsigned int col_idx, unsigned int row_idx);

  // columns, from settings. jobs keep their column by name, so renaming or removing one rewrites the jobs in it
  std::string columnNameProblem(std::string name, int skip = -1); // why a name can't be used ("" if it can), skip = the column being renamed
  void        addColumn(std::string title, size_t at);
  void        renameColumn(size_t idx, std::string title);
  void        removeColumn(size_t idx, size_t move_jobs_to); // the jobs in it go to another column
  void        moveColumn(size_t from, size_t to);
  void        setColumnHeader(size_t idx, termui::Style style);

  kan::Job *findJob(int id);
  int       nextJobId();

  void deleteJob(kan::Job job); // removes the job's file for good
  void archiveJob(kan::Job job);
  void restoreJob(kan::Job job); // back to its old column, or the first column if that's gone
  void finishJob(kan::Job job);
  void reopenJob(kan::Job job);  // back to the column it was finished from, or the first column if that's gone

  void loadBoard();
  void saveBoard();

  void loadJobs();
  void saveJobs();

  void loadJob();
  void saveJob(uint col, uint row);
  void saveJob(kan::Job);

  void reloadJobs();
  bool refresh(); // re-reads everything changed outside kan (eg by a git pull), true if the columns were replaced

  void               loadBranches();
  const git::Branch *branchFor(int id); // nullptr if the job has no branch

  void saveIssue(Issue issue);
  void loadIssues();

  void loadStyleMap();

  void loadContributors();
  bool isContributor(const User &user);
  void addContributor(User user);
  void saveContributors();
  void saveStyleMap();

};

} // namespace kan

#endif
