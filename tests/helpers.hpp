#ifndef KAN_TESTS_HELPERS_HPP
#define KAN_TESTS_HELPERS_HPP

#include <filesystem>
#include <format>
#include <kan/kan.hpp>
#include <string>
#include <unistd.h>

#include <gtest/gtest.h>

namespace testing_kan {

// a fresh folder under the temp dir, removed again at the end of the test
struct TempDir {
  std::filesystem::path path;

  explicit TempDir(const std::string &name) : path(std::filesystem::temp_directory_path() / std::format("kan-{}-{}", name, ::getpid())) {
    std::filesystem::remove_all(path);
    std::filesystem::create_directories(path);
  }

  ~TempDir() { std::filesystem::remove_all(path); }

  std::filesystem::path operator/(const std::string &p) const { return path / p; }
};

// runs the test from inside a folder - git, and so kan's git functions, work on the current directory
struct InDir {
  std::filesystem::path old = std::filesystem::current_path();

  explicit InDir(const std::filesystem::path &dir) { std::filesystem::current_path(dir); }
  ~InDir() { std::filesystem::current_path(old); }
};

inline void sh(const std::string &cmd) { ASSERT_EQ(std::system((cmd + " >/dev/null 2>&1").c_str()), 0) << cmd; }

// a git repo with one empty commit on main
inline void gitInit() { sh("git init -q -b main && git config user.name t && git config user.email t@t && git commit -q --allow-empty -m init"); }

// a board with the given columns in <dir>, loaded into the global kan::board (the board functions work on it)
inline void makeBoard(const std::filesystem::path &dir, std::vector<std::string> columns) {
  std::filesystem::create_directories(dir / "jobs");

  kan::Board b(dir);
  b.title = "Test";
  for (auto &t : columns) {
    kan::Column c;
    c.title = t;
    b.columns.push_back(c);
  }
  b.saveBoard();
}

inline void loadBoard(const std::filesystem::path &dir) {
  kan::board = kan::Board(dir);
  kan::board.loadBoard();
  kan::board.loadJobs();
}

inline kan::Job job(int id, std::string title, std::string status) {
  kan::Job j;
  j.id     = id;
  j.title  = title;
  j.status = status;
  return j;
}

} // namespace testing_kan

#endif
