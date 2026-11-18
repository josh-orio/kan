#include "helpers.hpp"

using namespace testing_kan;

class UndoTest : public ::testing::Test {
protected:
  TempDir dir{"undo"};

  void SetUp() override {
    makeBoard(dir.path, {"Backlog", "Doing", "Done"});
    loadBoard(dir.path);

    kan::board.saveJob(job(1, "first", "Backlog"));
    kan::board.reloadJobs();
  }

  void TearDown() override { kan::board = kan::Board(""); }

  kan::Job &first() { return *kan::board.findJob(1); }
};

TEST_F(UndoTest, NothingToUndo) {
  EXPECT_FALSE(kan::board.canUndo());
  EXPECT_EQ(kan::board.undo(), "Nothing to undo.");

  kan::board.beginUndo("nothing");
  kan::board.endUndo(); // no writes, so no step
  EXPECT_FALSE(kan::board.canUndo());
}

TEST_F(UndoTest, UndoesAnEdit) {
  kan::board.beginUndo("changes to #1");
  auto edited  = first();
  edited.title = "renamed";
  kan::board.saveJob(edited);
  kan::board.reloadJobs();
  kan::board.endUndo();

  ASSERT_EQ(first().title, "renamed");

  EXPECT_EQ(kan::board.undo(), "Undid changes to #1.");
  EXPECT_EQ(first().title, "first");
  EXPECT_FALSE(kan::board.canUndo());
}

TEST_F(UndoTest, UndoesNewJobs) {
  kan::board.beginUndo("new job #2");
  kan::board.saveJob(job(2, "second", "Backlog"));
  kan::board.endUndo();

  kan::board.undo();
  EXPECT_EQ(kan::board.findJob(2), nullptr);
}

TEST_F(UndoTest, UndoesDeleting) {
  kan::board.beginUndo("deleting #1");
  kan::board.deleteJob(first());
  kan::board.endUndo();
  ASSERT_EQ(kan::board.findJob(1), nullptr);

  kan::board.undo();
  ASSERT_NE(kan::board.findJob(1), nullptr);
  EXPECT_EQ(first().title, "first");
}

TEST_F(UndoTest, UndoesFinishingAndArchiving) {
  kan::board.beginUndo("finishing #1");
  kan::board.finishJob(first());
  kan::board.endUndo();

  kan::board.beginUndo("archiving #1");
  kan::board.archiveJob(*kan::board.findJob(1));
  kan::board.endUndo();

  ASSERT_EQ(kan::board.archive.size(), 1u);

  EXPECT_EQ(kan::board.undo(), "Undid archiving #1.");
  EXPECT_TRUE(kan::board.archive.empty());
  EXPECT_TRUE(first().finished);
  EXPECT_TRUE(kan::board.struck.contains(first().uuid)); // still struck through on the board

  EXPECT_EQ(kan::board.undo(), "Undid finishing #1.");
  EXPECT_FALSE(first().finished);
  EXPECT_FALSE(kan::board.struck.contains(first().uuid));
  EXPECT_EQ(kan::board.Jobs(0).size(), 1u);
}

TEST_F(UndoTest, OneStepCoversSeveralWrites) {
  kan::board.beginUndo("changes to #1");
  auto edited  = first();
  edited.title = "once";
  kan::board.saveJob(edited);
  edited.title = "twice";
  kan::board.saveJob(edited);
  kan::board.endUndo();

  kan::board.undo();
  kan::board.reloadJobs();
  EXPECT_EQ(first().title, "first"); // back to before the step, not the middle of it
}

TEST_F(UndoTest, WontOverwriteSomeoneElsesChange) {
  kan::board.beginUndo("changes to #1");
  auto edited  = first();
  edited.title = "mine";
  kan::board.saveJob(edited);
  kan::board.endUndo();

  // a git pull (or the cli) changes the job after that
  edited.title = "theirs";
  kan::Board(dir.path).saveJob(edited);

  EXPECT_EQ(kan::board.undo(), "Can't undo changes to #1 - the job has been changed since.");
  kan::board.reloadJobs();
  EXPECT_EQ(first().title, "theirs");
  EXPECT_FALSE(kan::board.canUndo()); // the step is dropped rather than tried again
}

TEST_F(UndoTest, WritesOutsideAStepArentRecorded) {
  auto edited  = first();
  edited.title = "unrecorded";
  kan::board.saveJob(edited);

  EXPECT_FALSE(kan::board.canUndo());
}
