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

  board = Board(_path); // create board instace pointing to path
  board.loadBoard();
  board.loadJobs();
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
  // nlohmann::json init_struct{{"title", "New Project"},
  //                            {"description", ""},
  //                            {"columns",
  //                             {{
  //                                  {"title", "Backlog"},
  //                                  {"uuid", UUIDv7::generate().to_string()},
  //                              },

  //                              {
  //                                  {"title", "Development"},
  //                                  {"uuid", UUIDv7::generate().to_string()},
  //                              },

  //                              {
  //                                  {"title", "Review"},
  //                                  {"uuid", UUIDv7::generate().to_string()},
  //                              },

  //                              {
  //                                  {"title", "Done"},
  //                                  {"uuid", UUIDv7::generate().to_string()},
  //                              }}}};
  // clang-format on

  kan::Board board=Board("");
  board.title = "New Project";

  // Column col1 = Column().title
  board.columns.push_back(Column(YAML::Load(R"({
    "title": "Backlog",
})")));

  board.columns.push_back(Column(YAML::Load(R"({
    "title": "Development",
})")));

  board.columns.push_back(Column(YAML::Load(R"({
    "title": "Review",
})")));

  board.columns.push_back(Column(YAML::Load(R"({
    "title": "Done",
})")));
  // clang-format off
  // board.fromJson(nlohmann::json{
  //   {"title", "New Project"},
  //   {"description", ""},
  //   {"columns",
  //     {{
  //       {"title", "Backlog"},
  //       // {"uuid", UUIDv7::generate().to_string()},
  //     },

  //     {
  //       {"title", "Development"},
  //       // {"uuid", UUIDv7::generate().to_string()},
  //     },

  //     {
  //       {"title", "Review"},
  //       // {"uuid", UUIDv7::generate().to_string()},
  //     },

  //     {
  //       {"title", "Done"},
  //       // {"uuid", UUIDv7::generate().to_string()},
  //     }}}});
  // // clang-format on

  // std::ofstream out_file(_path / "kan.json");//, std::ios::binary);
  std::ofstream out_file(_path / "kan.yaml");
  out_file << board.toYaml();

  // time_t timestamp;
  Timestamp timestamp;

  // nlohmann::json first_job_json{{"title", "First Job"},
  //                               {"description", "Welcome to KAN!"},
  //                               {"status", "Backlog"},
  //                               {"created", timestamp.toJson()},
  //                               {"updated", timestamp.toJson()},
  //                               {"creator", kan::User().getFromSystem().toJson()},
  //                               {"uuid", UUIDv7::generate().to_string()}};

  kan::Job first_job;
  first_job.title       = "First Job";
  first_job.description = "Welcome to KAN!";
  first_job.status      = "Backlog";
  
  first_job.history.addEvent({"Job created.",User().getFromSystem(), Timestamp().Now()});


  // first_job.fromJson(first_job_json);

  // std::ofstream out_file_job(_path / "jobs" / std::format("{}.bson", first_job.uuid), std::ios::binary);
  // nlohmann::json::to_bson(first_job.toJson(), out_file_job);
  std::ofstream out_file_job(_path / "jobs" / std::format("{}.yaml", first_job.uuid));
  // out_file_job << first_job.toJson().dump(2);
  out_file_job << first_job.toYaml();
    };

void KAN::start() {
  _renderer.show();
  // termui::TermSetups::fullscreen_no_mouse.configure();

  // state = State::Continue;
  // // bool reprint = true;

  // do {
  //   // if (reprint) {
  //   display();
  //   //   reprint = false;
  //   // }
  //   process_input();

  // } while (state == State::Continue);

  // termui::TermSetups::fullscreen.reset();
}





} // namespace kan
