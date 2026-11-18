#include "helpers.hpp"

using namespace testing_kan;

namespace {

constexpr time_t day = 24 * 60 * 60;
constexpr time_t now = 1'800'000'000;

kan::Timestamp at(time_t t) {
  kan::Timestamp ts;
  ts.time = t;
  return ts;
}

kan::User named(std::string name) {
  kan::User u;
  u.name = name;
  return u;
}

// a job created <created> days ago in Backlog
kan::Job made(int id, double created) {
  auto j    = job(id, std::format("job {}", id), "Backlog");
  j.created = at(now - static_cast<time_t>(created * day));
  return j;
}

void move(kan::Job &j, std::string to, double days_ago) {
  j.addEvent({std::format("Job status was updated: {} -> {}.", j.status, to), kan::User(), at(now - static_cast<time_t>(days_ago * day))});
  j.status = to;
}

const std::vector<std::string> columns = {"Backlog", "Doing", "Done"};

} // namespace

TEST(statusChangeTo, ReadsHistory) {
  using kan::stats::statusChangeTo;

  EXPECT_EQ(statusChangeTo("Job status was updated: Backlog -> In Review."), "In Review");
  EXPECT_EQ(statusChangeTo("Job status was updated: Old -> Done. The Old column was removed."), "Done");
  EXPECT_EQ(statusChangeTo("Job reopened to Doing."), "Doing");
  EXPECT_EQ(statusChangeTo("Job restored from the archive to Backlog."), "Backlog");
  EXPECT_EQ(statusChangeTo("Job priority was updated: Low -> High."), std::nullopt);
  EXPECT_EQ(statusChangeTo("Comment added."), std::nullopt);
}

TEST(stays, FollowsAJobAcrossColumns) {
  auto j = made(1, 10);
  move(j, "Doing", 7);
  move(j, "Done", 2);

  auto s = kan::stats::stays(j, now);

  ASSERT_EQ(s.size(), 3u);
  EXPECT_EQ(s[0].column, "Backlog");
  EXPECT_EQ(s[0].to - s[0].from, 3 * day);
  EXPECT_FALSE(s[0].open);
  EXPECT_EQ(s[1].column, "Doing");
  EXPECT_EQ(s[1].to - s[1].from, 5 * day);
  EXPECT_EQ(s[2].column, "Done");
  EXPECT_TRUE(s[2].open);
  EXPECT_EQ(s[2].to, now);
}

TEST(stays, EndsWhenFinished) {
  auto j = made(1, 4);
  move(j, "Doing", 3);
  j.finish(named("sam"), at(now - day));

  auto s = kan::stats::stays(j, now);

  ASSERT_EQ(s.size(), 2u);
  EXPECT_FALSE(s[1].open);
  EXPECT_EQ(s[1].to, now - day);
}

TEST(stays, ComesBackAfterReopening) {
  auto j = made(1, 4);
  j.finish(named("sam"), at(now - 3 * day));
  j.finished = false;
  j.addEvent({"Job reopened to Doing.", kan::User(), at(now - day)});
  j.status = "Doing";

  auto s = kan::stats::stays(j, now);

  ASSERT_EQ(s.size(), 2u);
  EXPECT_EQ(s[0].to, now - 3 * day); // the time spent finished doesn't count towards Backlog
  EXPECT_EQ(s[1].column, "Doing");
  EXPECT_EQ(s[1].from, now - day);
  EXPECT_TRUE(s[1].open);
}

TEST(build, CountsThroughputAndCycleTime) {
  std::vector<kan::Job> jobs;

  // finished 1, 2 and 9 weeks ago (the last is outside an 8 week window), taking 2, 4 and 6 days
  for (auto [id, weeks_ago, took] : std::vector<std::tuple<int, int, int>>{{1, 0, 2}, {2, 1, 4}, {3, 9, 6}}) {
    auto finished_at = now - weeks_ago * 7 * day - day;
    auto j           = made(id, 0);
    j.created        = at(finished_at - took * day);
    j.finish(named(id == 2 ? "ana" : "sam"), at(finished_at));
    jobs.push_back(j);
  }

  jobs.push_back(made(4, 3)); // still open

  auto r = kan::stats::build(columns, jobs, now, 8);

  EXPECT_EQ(r.finished, 2);
  EXPECT_EQ(r.finished_per_week, std::vector<int>({0, 0, 0, 0, 0, 0, 1, 1}));
  EXPECT_EQ(r.created, 3); // job 3 was made before the window
  ASSERT_TRUE(r.cycle_median_days);
  EXPECT_DOUBLE_EQ(*r.cycle_median_days, 3.0);

  ASSERT_EQ(r.finishers.size(), 2u);
  EXPECT_EQ(r.finishers[0].finished, 1);
}

TEST(build, TimeInColumns) {
  auto a = made(1, 10);
  move(a, "Doing", 8); // 2 days in Backlog
  move(a, "Done", 4);  // 4 days in Doing

  auto b = made(2, 6);
  move(b, "Doing", 2); // 4 days in Backlog, in Doing for 2 so far

  std::vector<kan::Job> jobs = {a, b}; // oldest points into this
  auto                  r    = kan::stats::build(columns, jobs, now, 8);

  ASSERT_EQ(r.columns.size(), 3u);
  EXPECT_EQ(r.columns[0].jobs_now, 0);
  EXPECT_DOUBLE_EQ(*r.columns[0].average_days, 3.0);
  EXPECT_EQ(r.columns[1].jobs_now, 1);
  EXPECT_DOUBLE_EQ(*r.columns[1].average_days, 4.0); // b hasn't left yet, so only a counts
  EXPECT_EQ(r.columns[2].jobs_now, 1);
  EXPECT_FALSE(r.columns[2].average_days);

  ASSERT_EQ(r.oldest.size(), 1u); // a is in the last column, where jobs belong
  EXPECT_EQ(r.oldest[0].first->id, 2);
  EXPECT_DOUBLE_EQ(r.oldest[0].second, 2.0);
}

TEST(build, EmptyBoard) {
  auto r = kan::stats::build(columns, {}, now, 4);

  EXPECT_EQ(r.finished_per_week, std::vector<int>({0, 0, 0, 0}));
  EXPECT_FALSE(r.cycle_median_days);
  EXPECT_TRUE(r.oldest.empty());
}

TEST(sparkline, Scales) {
  EXPECT_EQ(kan::stats::sparkline({0, 0}), "▁▁");
  EXPECT_EQ(kan::stats::sparkline({0, 4, 8}), "▁▅█");
}

TEST(days, Formats) {
  EXPECT_EQ(kan::stats::days(0.5), "12h");
  EXPECT_EQ(kan::stats::days(3.25), "3.2d");
  EXPECT_EQ(kan::stats::days(12.6), "13d");
}
