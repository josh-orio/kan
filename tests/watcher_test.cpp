#include "helpers.hpp"
#include <fstream>

using namespace testing_kan;

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
