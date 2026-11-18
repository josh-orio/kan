#include "helpers.hpp"

using namespace testing_kan;

TEST(Job, EventsMoveUpdatedOn) {
  auto j = job(1, "a", "Backlog");

  kan::User someone;
  someone.name = "someone";

  kan::Timestamp later = kan::Timestamp().Now();
  later.time += 60;

  j.addEvent({"Job title was updated: a -> b.", someone, later});

  EXPECT_EQ(j.updated.time, later.time);
  EXPECT_EQ(j.modifier.name, "someone");
}

TEST(Job, CommentsAndCommitLinksDontCountAsUpdates) {
  auto j      = job(1, "a", "Backlog");
  auto before = j.updated.time;

  kan::Timestamp later = kan::Timestamp().Now();
  later.time += 60;

  j.addEvent({"Comment added.", kan::User(), later}, false);

  EXPECT_EQ(j.updated.time, before);
  EXPECT_EQ(j.history.size(), 1u); // still in the history
}
