#include "helpers.hpp"
#include <map>

using namespace testing_kan;

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
  TempDir                                 dir("branches");
  std::map<std::string, kan::git::Branch> found;

  {
    InDir in(dir.path);

    gitInit();
    sh("git branch 1-fresh");
    sh("git switch -q -c 2-open && git commit -q --allow-empty -m work");
    sh("git switch -q -c 3-merged main && git commit -q --allow-empty -m done && git switch -q main && git merge -q --no-ff -m merge 3-merged");

    EXPECT_EQ(kan::git::defaultBranch(), "main");
    EXPECT_EQ(kan::git::switchBranch("1-fresh", false), "");
    EXPECT_NE(kan::git::switchBranch("1-fresh", true), ""); // already exists

    for (auto &b : kan::git::branches()) {
      found[b.name] = b;
    }
  }

  ASSERT_EQ(found.size(), 4u);
  EXPECT_TRUE(found["1-fresh"].current);
  EXPECT_FALSE(found["1-fresh"].merged);
  EXPECT_FALSE(found["2-open"].merged);
  EXPECT_TRUE(found["3-merged"].merged);
  EXPECT_FALSE(found["main"].merged);
}
