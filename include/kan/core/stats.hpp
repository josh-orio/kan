#ifndef CORE_STATS_HPP
#define CORE_STATS_HPP

#include <ctime>
#include <kan/core/job.hpp>
#include <optional>
#include <string>
#include <vector>

namespace kan {
namespace stats {

// where a job has been: each column it entered and when, read back from its history
struct Stay {
  std::string column;
  time_t      from, to; // to is when it left, finished or was archived - now if it's still there
  bool        open;     // still there
};

std::vector<Stay> stays(const Job &job, time_t now);

std::optional<std::string> statusChangeTo(const std::string &event); // "Job status was updated: A -> B." -> B, also reopens and restores

struct ColumnStats {
  std::string           column;
  int                   jobs_now = 0;
  std::optional<double> average_days; // over the stays that ended in the window
  int                   left = 0;     // how many stays ended in the window
};

struct Finisher {
  std::string name;
  int         finished;
};

struct Report {
  int    weeks;
  time_t now;

  std::vector<int> finished_per_week; // oldest first, the last entry is the 7 days up to now
  int              created  = 0;      // in the window
  int              finished = 0;

  std::optional<double> cycle_median_days; // created -> finished, jobs finished in the window
  std::optional<double> cycle_p85_days;    // 85% of jobs were done in this many days or fewer

  std::vector<ColumnStats>                    columns;   // in board order
  std::vector<Finisher>                       finishers; // most first
  std::vector<std::pair<const Job *, double>> oldest;    // open jobs that have sat in their column longest, with days there
};

// jobs = everything, on the board, finished and archived. oldest points into it
Report build(const std::vector<std::string> &columns, const std::vector<Job> &jobs, time_t now, int weeks = 8);

std::string days(double d); // 0.4 -> "9h", 3.25 -> "3.3d"

std::string sparkline(const std::vector<int> &values); // ▁▃▂█, scaled to the biggest value

} // namespace stats
} // namespace kan

#endif
