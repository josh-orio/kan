#include "helpers.hpp"

using namespace testing_kan;

// a real board in a temp folder - the board functions work on the global kan::board
class BoardTest : public ::testing::Test {
protected:
  TempDir dir{"board"};

  void TearDown() override { kan::board = kan::Board(""); }

  void load() { loadBoard(dir.path); }
};

TEST_F(BoardTest, RenamesAndRemovesColumns) {
  makeBoard(dir.path, {"Backlog", "Review", "Done"});

  {
    kan::Board b(dir.path);
    auto       z = job(2, "z", "Review");
    z.finish(kan::User(), kan::Timestamp().Now());
    b.saveJob(job(1, "a", "Review"));
    b.saveJob(z);
  }

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
  kan::board.saveJob(job(3, "lost", "Nowhere"));
  load();
  EXPECT_EQ(kan::board.Jobs(0).size(), 1u);
}

TEST_F(BoardTest, AllJobsListsEachJobOnce) {
  makeBoard(dir.path, {"Backlog", "Done"});
  load();

  kan::board.saveJob(job(1, "open", "Backlog"));
  kan::board.saveJob(job(2, "archived", "Backlog"));
  kan::board.saveJob(job(3, "finished", "Backlog"));
  kan::board.reloadJobs();

  kan::board.archiveJob(*kan::board.findJob(2));
  kan::board.finishJob(*kan::board.findJob(3)); // struck - on the board and in the finished log

  EXPECT_EQ(kan::board.allJobs().size(), 3u);
}
