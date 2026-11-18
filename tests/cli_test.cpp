#include "helpers.hpp"

using namespace testing_kan;

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
