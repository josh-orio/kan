#include <kan/kan.hpp>

#include <filesystem>
#include <fstream>

namespace kan {

KAN::KAN(std::string path) : _path(), /*_board(),*/ _renderer(board) {
  std::filesystem::path dir(path);

  _path = dir / ".kan";

  // std::cout << _path << std::endl;

  if (!std::filesystem::exists(_path)) {
    init();
  }

  loadBoard();
  loadJobs();
}

void KAN::init() {
  /*
  creates directories, and files to prepopulate the system
  */

  // create some directories
  std::filesystem::create_directory(_path);
  std::filesystem::create_directory(_path / "jobs");
  std::filesystem::create_directory(_path / "issues");

  // create files to prepopulate the system

  // board

  // clang-format -off
  nlohmann::json init_struct{{"title", "New Project"},
                             {"description", ""},
                             {"columns",
                              {{
                                   {"title", "Backlog"},
                                   {"uuid", UUIDv7::generate().to_string()},
                               },

                               {
                                   {"title", "Development"},
                                   {"uuid", UUIDv7::generate().to_string()},
                               },

                               {
                                   {"title", "Review"},
                                   {"uuid", UUIDv7::generate().to_string()},
                               },

                               {
                                   {"title", "Done"},
                                   {"uuid", UUIDv7::generate().to_string()},
                               }}}};
  // clang-format on

  // std::ofstream out_file(_path / "kan.json");//, std::ios::binary);
  std::ofstream out_file(_path / "kan.json");
  out_file << init_struct.dump(2);

  // time_t timestamp;
  Timestamp timestamp;

  nlohmann::json first_job_json{{"title", "First Job"},
                                {"description", "Welcome to KAN!"},
                                {"status", "Backlog"},
                                {"created", timestamp.toJson()},
                                {"updated", timestamp.toJson()},
                                {"creator", kan::User().getFromSystem().toJson()},
                                {"uuid", UUIDv7::generate().to_string()}};

  kan::Job first_job;
  first_job.fromJson(first_job_json);

  // std::ofstream out_file_job(_path / "jobs" / std::format("{}.bson", first_job.uuid), std::ios::binary);
  // nlohmann::json::to_bson(first_job.toJson(), out_file_job);
  std::ofstream out_file_job(_path / "jobs" / std::format("{}.json", first_job.uuid));
  out_file_job << first_job_json.dump(2);
};

void KAN::start() {
  termui::TermSetups::fullscreen_no_mouse.configure();

  state = State::Continue;
  // bool reprint = true;

  do {
    // if (reprint) {
    display();
    //   reprint = false;
    // }
    process_input();

  } while (state == State::Continue);

  termui::TermSetups::fullscreen.reset();
}

void KAN::display() {
  internal_update();

  termui::terminal.ClearScreen();
  termui::terminal.ClearScrollback();

  termui::terminal.MoveCursor(1, 2);
  _renderer.render();

  termui::terminal.flush();
}

void KAN::process_input() {
  std::string inp = termui::terminal.read();

  if (inp == "q") {
    state = termui::Interface::State::Exit;
    // serialize board out to disk
  }
  else if (inp == termui::key::U_ARROW) {
    _renderer.Up();
  }
  else if (inp == termui::key::D_ARROW) {
    _renderer.Down();
  }
  else if (inp == termui::key::L_ARROW) {
    _renderer.Left();
  }
  else if (inp == termui::key::R_ARROW) {
    _renderer.Right();
  }
  else if (inp == "n") {
    auto [success, new_job] = tui::NewJob().show();

    if (success) {
      new_job.status = board.Column(_renderer.ActiveColumn()).Title(); // board._columns.at(_renderer.ActiveColumn())._title;

      board.Column(_renderer.ActiveColumn()).Jobs().push_back(new_job); // board._columns.at(_renderer.ActiveColumn())._jobs.push_back(new_job);

      saveJob(new_job);
    }

    state = termui::Interface::State::Continue;
  }
  else if (inp == "o" || inp == termui::key::ENTER) {

    // seralize job out to disk

    auto [modified, job] = tui::JobWindow(board.Job(_renderer.ActiveColumn(), _renderer.ActiveRow()))
                               .show(); // board._columns.at(_renderer.ActiveColumn())._jobs.at(_renderer.ActiveRow())).show();

    // if modified
    if (modified) {
      saveJob(job);
      reloadJobs();
    }

    state = termui::Interface::State::Continue;
  }
  else if (inp == "s") {

    state = termui::Interface::State::Continue;
  }

  else if (inp == "?") {
    // help page
    state = termui::Interface::State::Continue;
  }
}

void KAN::internal_update() { _renderer.height(termui::terminal.height()).width(termui::terminal.width()); }

void KAN::loadBoard() {

  //  std::ifstream  in_file(_path / "kan.bson", std::ios::binary);
  // nlohmann::json j_loaded = nlohmann::json::from_bson(in_file);

  std::ifstream  in_file(_path / "kan.json");
  nlohmann::json j_loaded = nlohmann::json::parse(in_file);

  board.fromJson(j_loaded);
}

void KAN::saveBoard() {
  auto json = board.toJson();
  auto bson = nlohmann::json::to_bson(json);

  std::ofstream out_file(_path / "kan.bson", std::ios::binary);
  out_file.write(reinterpret_cast<const char *>(bson.data()), bson.size());
  out_file.close();
}

void KAN::loadJobs() {
  kan::Job loaded_job;

  for (const auto &entry : std::filesystem::directory_iterator(_path / "jobs")) {

    // std::cout << entry.path() << std::endl;

    // try {
    std::ifstream in_file = std::ifstream(entry.path());

    loaded_job.fromJson(nlohmann::json::parse(in_file));

    for (auto &col : board.Columns()) {
      if (col.Title() == loaded_job.status) {
        // col._jobs.push_back(loaded_job);
        // board.
        col.Jobs().push_back(loaded_job);
      }
    }

    // } catch (std::exception ex) {
    //   std::cout << ex.what() << std::endl;
    // }
  }

  // sort all the jobs oldest highest - newest lowest

  for (auto &col : board.Columns()) {
    std::sort(col.Jobs().begin(), col.Jobs().end(), [](const kan::Job &a, const kan::Job &b) { return a.created.time < b.created.time; });
  }
}

// void KAN::saveJobs() {
//   // save all board files - probably not needed
// }

// void KAN::loadJob() {
//   // how would you load the file without knowing its name
//   // why would you even load an individual job?
// }

void KAN::saveJob(uint col, uint row) {
  auto job = board.Column(col).Job(row);

  // std::ofstream out_file_job(_path / "jobs" / std::format("{}.bson", first_job.uuid), std::ios::binary);
  // nlohmann::json::to_bson(first_job.toJson(), out_file_job);
  std::ofstream out_file_job(_path / "jobs" / std::format("{}.json", job.uuid));
  out_file_job << job.toJson().dump(2);
}

void KAN::saveJob(kan::Job job) {
  // auto job = _board._columns.at(col)._jobs.at(row);

  // std::ofstream out_file_job(_path / "jobs" / std::format("{}.bson", first_job.uuid), std::ios::binary);
  // nlohmann::json::to_bson(first_job.toJson(), out_file_job);
  std::ofstream out_file_job(_path / "jobs" / std::format("{}.json", job.uuid));
  out_file_job << job.toJson().dump(2);
}

void KAN::reloadJobs() {
  for (auto &col : board.Columns()) {
    col.Jobs().clear();
  }

  loadJobs();
  // for (auto col : board.Columns()) {
  //   for (auto job : col.Jobs()) {
  //     if (job.status != col.Title()) {
  //       for (auto col2 : board.Columns()) {
  //         if (col2.Title() == job.status) {
  //           col2.Jobs().push_back(job);
  //           // col.Jobs().erase(job);
  //         }
  //       }
  //     }
  //   }
  // }

  // for (auto &col : board.Columns()) {
  //   auto &jobs = col.Jobs();
  //   jobs.erase(std::remove_if(jobs.begin(), jobs.end(), [&](const Job &j) { return j.status != col.Title(); }), jobs.end());
  // }
}

} // namespace kan
