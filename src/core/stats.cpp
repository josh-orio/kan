#include <algorithm>
#include <cmath>
#include <format>
#include <kan/core.hpp>
#include <map>

namespace kan {
namespace stats {

constexpr double day = 24 * 60 * 60;

std::optional<std::string> statusChangeTo(const std::string &event) {
  static const std::vector<std::pair<std::string, std::string>> forms = {
      {"Job status was updated: ", " -> "}, // "Job status was updated: A -> B." and "... -> B. The A column was removed."
      {"Job reopened to ", ""},
      {"Job restored from the archive to ", ""},
  };

  for (auto &[prefix, arrow] : forms) {
    if (!event.starts_with(prefix)) {
      continue;
    }

    auto start = prefix.size();
    if (!arrow.empty()) {
      auto at = event.find(arrow, start);
      if (at == std::string::npos) {
        return std::nullopt;
      }
      start = at + arrow.size();
    }

    auto end = event.find(". ", start);
    auto to  = event.substr(start, end == std::string::npos ? std::string::npos : end - start);

    if (to.ends_with(".")) {
      to.pop_back();
    }

    return to.empty() ? std::nullopt : std::optional(to);
  }

  return std::nullopt;
}

static std::optional<std::string> statusChangeFrom(const std::string &event) {
  static const std::string prefix = "Job status was updated: ";

  if (!event.starts_with(prefix)) {
    return std::nullopt;
  }

  auto at = event.find(" -> ", prefix.size());
  return at == std::string::npos ? std::nullopt : std::optional(event.substr(prefix.size(), at - prefix.size()));
}

std::vector<Stay> stays(const Job &job, time_t now) {
  auto events = job.history.events;
  std::stable_sort(events.begin(), events.end(), [](const Event &a, const Event &b) { return a.time.time < b.time.time; });

  // the column it was created in is wherever the first move came from
  std::string first = job.finished ? job.finished_from : job.status;
  for (auto &e : events) {
    if (auto from = statusChangeFrom(e.text)) {
      first = *from;
      break;
    }
  }

  std::vector<Stay> out = {{first, job.created.time, now, true}};

  // the job is off the board between finishing/archiving and coming back
  auto leave = [&](time_t at) {
    if (out.back().open) {
      out.back().to   = at;
      out.back().open = false;
    }
  };

  for (auto &e : events) {
    const std::string &text = e.text;

    if (auto to = statusChangeTo(text)) {
      leave(e.time.time);
      out.push_back({*to, e.time.time, now, true});
    }
    else if (text.starts_with("Job finished") || text == "Job archived.") {
      leave(e.time.time);
    }
  }

  // older jobs may not have a finish event, but they do have finished_at
  if (job.finished) {
    leave(job.finished_at.time);
  }
  else if (job.archived) {
    leave(job.updated.time);
  }

  return out;
}

static double percentile(std::vector<double> v, double p) {
  std::sort(v.begin(), v.end());

  auto rank = p * (v.size() - 1);
  auto lo   = static_cast<size_t>(std::floor(rank));
  auto hi   = static_cast<size_t>(std::ceil(rank));

  return v[lo] + (v[hi] - v[lo]) * (rank - lo);
}

Report build(const std::vector<std::string> &columns, const std::vector<Job> &jobs, time_t now, int weeks) {
  Report r;
  r.weeks = weeks;
  r.now   = now;
  r.finished_per_week.assign(weeks, 0);

  time_t window_start = now - static_cast<time_t>(weeks * 7 * day);

  std::vector<double>                cycle;
  std::map<std::string, Finisher>    finishers;
  std::map<std::string, ColumnStats> by_column;
  std::map<std::string, double>      column_total;

  for (auto &c : columns) {
    by_column[c] = {c};
  }

  for (auto &job : jobs) {
    if (job.created.time >= window_start && job.created.time <= now) {
      r.created++;
    }

    if (job.finished && job.finished_at.time >= window_start && job.finished_at.time <= now) {
      r.finished++;

      auto week = static_cast<int>((now - job.finished_at.time) / (7 * day));
      r.finished_per_week[std::clamp(weeks - 1 - week, 0, weeks - 1)]++;

      cycle.push_back(std::max(0.0, (job.finished_at.time - job.created.time) / day));

      auto finisher = job.finisher;
      auto who      = finisher.name.empty() ? std::string("Someone") : finisher.Name();
      finishers.try_emplace(who, Finisher{who, 0}).first->second.finished++;
    }

    for (auto &stay : stays(job, now)) {
      auto it = by_column.find(stay.column);
      if (it == by_column.end()) { // a column that's since been removed
        continue;
      }

      if (stay.open && !job.finished && !job.archived) {
        it->second.jobs_now++;

        if (stay.column != columns.back()) { // sitting in the last column is where jobs are meant to end up
          r.oldest.push_back({&job, (now - stay.from) / day});
        }
      }
      else if (!stay.open && stay.to >= window_start) {
        it->second.left++;
        column_total[stay.column] += (stay.to - stay.from) / day;
      }
    }
  }

  if (!cycle.empty()) {
    r.cycle_median_days = percentile(cycle, 0.5);
    r.cycle_p85_days    = percentile(cycle, 0.85);
  }

  for (auto &c : columns) {
    auto s = by_column[c];
    if (s.left > 0) {
      s.average_days = column_total[c] / s.left;
    }
    r.columns.push_back(s);
  }

  for (auto &[_, f] : finishers) {
    r.finishers.push_back(f);
  }
  std::stable_sort(r.finishers.begin(), r.finishers.end(), [](const Finisher &a, const Finisher &b) { return a.finished > b.finished; });

  std::stable_sort(r.oldest.begin(), r.oldest.end(), [](const auto &a, const auto &b) { return a.second > b.second; });
  if (r.oldest.size() > 5) {
    r.oldest.resize(5);
  }

  return r;
}

std::string days(double d) {
  if (d < 1) {
    return std::format("{}h", static_cast<int>(std::round(d * 24)));
  }
  if (d < 10) {
    return std::format("{:.1f}d", d);
  }
  return std::format("{}d", static_cast<int>(std::round(d)));
}

std::string sparkline(const std::vector<int> &values) {
  static const std::vector<std::string> bars = {"▁", "▂", "▃", "▄", "▅", "▆", "▇", "█"};

  int top = values.empty() ? 0 : *std::max_element(values.begin(), values.end());

  std::string out;
  for (int v : values) {
    out += top == 0 ? bars.front() : bars[(v * (bars.size() - 1) + top / 2) / top];
  }
  return out;
}

} // namespace stats
} // namespace kan
