#include "helpers.hpp"
#include <fstream>

using namespace testing_kan;

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

class InstallHook : public ::testing::Test {
protected:
  TempDir dir{"hook"};
  InDir   in{dir.path};

  std::string hook() {
    std::ifstream in(dir / ".git/hooks/post-commit");
    return std::string(std::istreambuf_iterator<char>(in), {});
  }
};

TEST_F(InstallHook, AddsAHook) {
  gitInit();

  EXPECT_NE(kan::hooks::installHook(dir.path), "");
  EXPECT_TRUE(hook().starts_with("#!/bin/sh\n"));
  EXPECT_NE(hook().find("kan link-commit"), std::string::npos);

  auto perms = std::filesystem::status(dir / ".git/hooks/post-commit").permissions();
  EXPECT_NE(perms & std::filesystem::perms::owner_exec, std::filesystem::perms::none);
}

TEST_F(InstallHook, OnlyOnce) {
  gitInit();

  kan::hooks::installHook(dir.path);
  auto first = hook();

  EXPECT_EQ(kan::hooks::installHook(dir.path), "");
  EXPECT_EQ(hook(), first);
}

TEST_F(InstallHook, KeepsAnExistingShellHook) {
  gitInit();
  std::ofstream(dir / ".git/hooks/post-commit") << "#!/bin/sh\necho mine\n";

  kan::hooks::installHook(dir.path);

  EXPECT_TRUE(hook().starts_with("#!/bin/sh\necho mine\n"));
  EXPECT_NE(hook().find("kan link-commit"), std::string::npos);
}

TEST_F(InstallHook, LeavesOtherLanguagesAlone) {
  gitInit();
  std::ofstream(dir / ".git/hooks/post-commit") << "#!/usr/bin/env python3\nprint('mine')\n";

  EXPECT_NE(kan::hooks::installHook(dir.path).find("isn't a shell script"), std::string::npos);
  EXPECT_EQ(hook(), "#!/usr/bin/env python3\nprint('mine')\n");
}

TEST_F(InstallHook, FollowsHooksPath) {
  gitInit();
  sh("git config core.hooksPath .githooks");

  kan::hooks::installHook(dir.path);

  EXPECT_TRUE(std::filesystem::exists(dir / ".githooks/post-commit"));
}

TEST_F(InstallHook, NothingOutsideARepo) { EXPECT_EQ(kan::hooks::installHook(dir.path), ""); }
