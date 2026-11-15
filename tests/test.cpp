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
