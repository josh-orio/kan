#include <kan/kan.hpp>
#include <array>
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
