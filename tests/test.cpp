#include <kan/kan.hpp>
#include <array>
#include <filesystem>
#include <format>
#include <fstream>
#include <map>
#include <unistd.h>
#include <gtest/gtest.h>

TEST(library, Test1) {
  EXPECT_EQ(1,1);
}

using ids = std::vector<int>;

TEST(parseJobRefs, FindsReferences) {
  EXPECT_EQ(kan::hooks::parseJobRefs("fix #1"), ids({1}));
  EXPECT_EQ(kan::hooks::parseJobRefs("#12 at the start"), ids({12}));
  EXPECT_EQ(kan::hooks::parseJobRefs("fix #1, #22 and (#3)\n\ncloses #4."), ids({1, 22, 3, 4}));
}

TEST(parseJobRefs, DropsRepeats) { EXPECT_EQ(kan::hooks::parseJobRefs("#2 #1 #2"), ids({2, 1})); }

TEST(parseJobRefs, IgnoresNonReferences) {
  EXPECT_EQ(kan::hooks::parseJobRefs(""), ids({}));
  EXPECT_EQ(kan::hooks::parseJobRefs("no refs here"), ids({}));
  EXPECT_EQ(kan::hooks::parseJobRefs("# #"), ids({}));
  EXPECT_EQ(kan::hooks::parseJobRefs("#"), ids({}));
  EXPECT_EQ(kan::hooks::parseJobRefs("abc#12"), ids({}));
  EXPECT_EQ(kan::hooks::parseJobRefs("#12abc"), ids({}));
  EXPECT_EQ(kan::hooks::parseJobRefs("#0"), ids({}));
  EXPECT_EQ(kan::hooks::parseJobRefs("#99999999999999999999"), ids({}));
}

TEST(parseClosingRefs, FindsKeywords) {
  EXPECT_EQ(kan::hooks::parseClosingRefs("fixes #12"), ids({12}));
  EXPECT_EQ(kan::hooks::parseClosingRefs("Closes #3"), ids({3}));
  EXPECT_EQ(kan::hooks::parseClosingRefs("resolved: #4."), ids({4}));
  EXPECT_EQ(kan::hooks::parseClosingRefs("add login\n\nfinishes #5"), ids({5}));
  EXPECT_EQ(kan::hooks::parseClosingRefs("fix #1"), ids({1}));
}

TEST(parseClosingRefs, FindsLists) {
  EXPECT_EQ(kan::hooks::parseClosingRefs("fixes #1, #2 and #3"), ids({1, 2, 3}));
  EXPECT_EQ(kan::hooks::parseClosingRefs("fixes #1 closes #2 fixes #1"), ids({1, 2}));
}

TEST(parseClosingRefs, IgnoresPlainRefs) {
  EXPECT_EQ(kan::hooks::parseClosingRefs("see #12"), ids({}));
  EXPECT_EQ(kan::hooks::parseClosingRefs("fixes the bug in #12"), ids({}));
  EXPECT_EQ(kan::hooks::parseClosingRefs("prefixes #12"), ids({}));
  EXPECT_EQ(kan::hooks::parseClosingRefs("fixes #12abc"), ids({}));
  EXPECT_EQ(kan::hooks::parseClosingRefs(""), ids({}));
}

TEST(jobBranchName, Slugs) {
  EXPECT_EQ(kan::git::jobBranchName(12, "Fix login crash!"), "12-fix-login-crash");
  EXPECT_EQ(kan::git::jobBranchName(3, "  API: v2 / auth  "), "3-api-v2-auth");
  EXPECT_EQ(kan::git::jobBranchName(4, "???"), "4");
  EXPECT_EQ(kan::git::jobBranchName(5, "café menu"), "5-caf-menu");
  EXPECT_EQ(kan::git::jobBranchName(6, "one two three four five six seven eight nine"), "6-one-two-three-four-five-six-seven-eight");
}

TEST(branchJobId, ReadsIds) {
  EXPECT_EQ(kan::git::branchJobId("12-fix-login-crash"), 12);
  EXPECT_EQ(kan::git::branchJobId("josh/7-tidy"), 7);
  EXPECT_EQ(kan::git::branchJobId("9"), 9);
  EXPECT_EQ(kan::git::branchJobId("main"), 0);
  EXPECT_EQ(kan::git::branchJobId("v2-release"), 0);
  EXPECT_EQ(kan::git::branchJobId("12abc"), 0);
  EXPECT_EQ(kan::git::branchJobId("0-zero"), 0);
}

// runs git for real in a throwaway repo
TEST(branches, OpenAndMerged) {
  auto old = std::filesystem::current_path();
  auto dir = std::filesystem::temp_directory_path() / std::format("kan-test-{}", ::getpid());

  std::filesystem::create_directories(dir);
  std::filesystem::current_path(dir);

  auto sh = [](const std::string &cmd) { ASSERT_EQ(std::system((cmd + " >/dev/null 2>&1").c_str()), 0) << cmd; };

  sh("git init -q -b main && git config user.name t && git config user.email t@t && git commit -q --allow-empty -m init");
  sh("git branch 1-fresh");
  sh("git switch -q -c 2-open && git commit -q --allow-empty -m work");
  sh("git switch -q -c 3-merged main && git commit -q --allow-empty -m done && git switch -q main && git merge -q --no-ff -m merge 3-merged");

  EXPECT_EQ(kan::git::defaultBranch(), "main");
  EXPECT_EQ(kan::git::switchBranch("1-fresh", false), "");
  EXPECT_NE(kan::git::switchBranch("1-fresh", true), ""); // already exists

  std::map<std::string, kan::git::Branch> found;
  for (auto &b : kan::git::branches()) {
    found[b.name] = b;
  }

  std::filesystem::current_path(old);
  std::filesystem::remove_all(dir);

  ASSERT_EQ(found.size(), 4u);
  EXPECT_TRUE(found["1-fresh"].current);
  EXPECT_FALSE(found["1-fresh"].merged);
  EXPECT_FALSE(found["2-open"].merged);
  EXPECT_TRUE(found["3-merged"].merged);
  EXPECT_FALSE(found["main"].merged);
}

TEST(Watcher, SeesChanges) {
  auto dir = std::filesystem::temp_directory_path() / std::format("kan-watch-{}", ::getpid());
  std::filesystem::create_directories(dir);

  auto write = [&](const std::string &name, const std::string &text) { std::ofstream(dir / name) << text; };

  write("a.yaml", "one");

  kan::Watcher watcher({dir, dir / "missing.yaml"});
  EXPECT_FALSE(watcher.changed());

  write("a.yaml", "two!"); // rewritten
  EXPECT_TRUE(watcher.changed());
  EXPECT_FALSE(watcher.changed());

  write("b.yaml", "new"); // added
  EXPECT_TRUE(watcher.changed());

  std::filesystem::remove(dir / "a.yaml"); // removed
  EXPECT_TRUE(watcher.changed());

  write("missing.yaml", "here now");
  EXPECT_TRUE(watcher.changed());

  watcher.reset();
  EXPECT_TRUE(watcher.changed());

  std::filesystem::remove_all(dir);
}

TEST(matchColumn, MatchesNamesAndPrefixes) {
  std::vector<std::string> cols = {"Backlog", "Development", "In Review", "Done"};

  EXPECT_EQ(kan::cli::matchColumn(cols, "done"), 3);
  EXPECT_EQ(kan::cli::matchColumn(cols, "DEV"), 1);
  EXPECT_EQ(kan::cli::matchColumn(cols, "in review"), 2);
  EXPECT_EQ(kan::cli::matchColumn(cols, "b"), 0);
  EXPECT_THROW(kan::cli::matchColumn(cols, "d"), std::runtime_error); // Development or Done
  EXPECT_THROW(kan::cli::matchColumn(cols, "qa"), std::runtime_error);
  EXPECT_THROW(kan::cli::matchColumn(cols, ""), std::runtime_error);
}

TEST(styling, ParsesAndNamesColours) {
  using kan::styling::colorName;
  using kan::styling::parseColor;

  EXPECT_EQ(parseColor("default"), termui::Color::Inherit());
  EXPECT_EQ(parseColor(""), termui::Color::Inherit());
  EXPECT_EQ(parseColor("Red"), termui::Color(uint8_t(1)));
  EXPECT_EQ(parseColor("grey"), termui::Color(uint8_t(8)));
  EXPECT_EQ(parseColor("208"), termui::Color(uint8_t(208)));
  EXPECT_EQ(parseColor("#FF8800"), termui::Color(uint8_t(255), uint8_t(136), uint8_t(0)));
  EXPECT_EQ(parseColor("256"), std::nullopt);
  EXPECT_EQ(parseColor("#ff88"), std::nullopt);
  EXPECT_EQ(parseColor("#gg0000"), std::nullopt);
  EXPECT_EQ(parseColor("reddish"), std::nullopt);

  for (std::string name : {"default", "black", "bright-cyan", "208", "#ff8800"}) {
    EXPECT_EQ(colorName(*parseColor(name)), name);
  }
}

TEST(styling, ColumnHeaderRoundTrips) {
  kan::Column column;
  column.title  = "Review";
  column.header = termui::Style(termui::Color(uint8_t(3)), termui::Color("#202020"), {termui::SGR::Italic, termui::SGR::Underlined});

  auto yaml = column.toYaml();
  EXPECT_EQ(yaml["header"]["foreground"].as<std::string>(), "yellow");
  EXPECT_EQ(yaml["header"]["background"].as<std::string>(), "#202020");

  EXPECT_EQ(kan::Column(yaml).header, column.header);

  kan::Column plain;
  plain.title = "Backlog";
  EXPECT_FALSE(plain.toYaml()["header"]); // the default isn't written out
  EXPECT_EQ(kan::Column(plain.toYaml()).header, kan::Column::defaultHeader());
}

// a real board in a temp folder - the board functions work on the global kan::board
TEST(Board, RenamesAndRemovesColumns) {
  auto dir = std::filesystem::temp_directory_path() / std::format("kan-cols-{}", ::getpid());
  std::filesystem::create_directories(dir / "jobs");

  {
    kan::Board b(dir);
    b.title = "Test";
    for (auto t : {"Backlog", "Review", "Done"}) {
      kan::Column c;
      c.title = t;
      b.columns.push_back(c);
    }
    b.saveBoard();

    kan::Job a, z;
    a.title = "a", a.status = "Review", a.id = 1;
    z.title = "z", z.status = "Review", z.id = 2;
    z.finish(kan::User(), kan::Timestamp().Now());
    b.saveJob(a);
    b.saveJob(z);
  }

  auto load = [&] {
    kan::board = kan::Board(dir);
    kan::board.loadBoard();
    kan::board.loadJobs();
  };

  load();
  EXPECT_EQ(kan::board.columnNameProblem("review"), "There's already a column called Review.");
  EXPECT_EQ(kan::board.columnNameProblem("Review", 1), "");
  EXPECT_EQ(kan::board.columnNameProblem("  "), "A column needs a name.");

  kan::board.renameColumn(1, "In Review");
  load();
  EXPECT_EQ(kan::board.Statuses(), std::vector<std::string>({"Backlog", "In Review", "Done"}));
  EXPECT_EQ(kan::board.Jobs(1).size(), 1u);
  EXPECT_EQ(kan::board.finished.at(0).finished_from, "In Review");

  kan::board.addColumn("Testing", 2);
  kan::board.moveColumn(2, 0);
  load();
  EXPECT_EQ(kan::board.Statuses(), std::vector<std::string>({"Testing", "Backlog", "In Review", "Done"}));

  kan::board.removeColumn(2, 3); // In Review -> Done
  load();
  EXPECT_EQ(kan::board.Statuses(), std::vector<std::string>({"Testing", "Backlog", "Done"}));
  EXPECT_EQ(kan::board.Jobs(2).size(), 1u);
  EXPECT_EQ(kan::board.Jobs(2).at(0).status, "Done");
  EXPECT_EQ(kan::board.finished.at(0).finished_from, "Done");

  // a job whose column is gone (renamed on another branch, say) still shows up, in the first column
  kan::Job lost;
  lost.title = "lost", lost.status = "Nowhere", lost.id = 3;
  kan::board.saveJob(lost);
  load();
  EXPECT_EQ(kan::board.Jobs(0).size(), 1u);

  kan::board = kan::Board("");
  std::filesystem::remove_all(dir);
}
