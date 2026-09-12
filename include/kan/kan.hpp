#ifndef KAN_HPP
#define KAN_HPP

#include <nlohmann/json.hpp>
#include <string>

#include <kan/core.hpp>
#include <kan/tui.hpp>

namespace kan {

class Status {};

class KAN : termui::Interface {
private:
  std::filesystem::path _path;
  kan::tui::Board       _renderer;

  void init();

  void display();
  void process_input();
  void internal_update();

  void loadBoard();
  void saveBoard();

  void loadJobs();
  void saveJobs();

  void loadJob();
  void saveJob(uint col, uint row);
  void saveJob(kan::Job);

  void reloadJobs();

public:
  KAN(std::string path);

  void start();
};

} // namespace kan

#endif
