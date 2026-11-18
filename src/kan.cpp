#include <kan/kan.hpp>

#include <filesystem>
#include <fstream>

namespace kan {

namespace scripts {
constexpr char setup_py[] = {
#embed "../scripts/setup.py"
};

constexpr char post_commit_py[] = {
#embed "../scripts/post-commit.py"
};
}; // namespace scripts

KAN::KAN(std::string path) : _path() {
  std::filesystem::path dir(path);

  _path = dir / ".kan";

  if (!std::filesystem::exists(_path)) {
    init();
  }

  board = Board(_path); // create board instace pointing to path
  board.loadBoard();
  board.loadJobs();
  board.loadStyleMap();
  board.loadContributors();
  board.loadBranches();

  board.styleMap.add("default", termui::Style(termui::terminal.ForegroundColor(), termui::terminal.BackgroundColor()));

  // shifts the terminal background away from black/white so it stands out
  auto offsetColor = [](int amount) {
    auto color = termui::terminal.BackgroundColor();

    int tamper = amount * (termui::terminal.HasDarkBackground() ? 1 : -1);

    color.rgb.r += tamper;
    color.rgb.g += tamper;
    color.rgb.b += tamper;

    return color;
  };

  board.styleMap.add("highlight", termui::Style(termui::Color::Inherit(), offsetColor(15)));
  board.styleMap.add("column_separator", termui::Style(offsetColor(60), termui::Color::Inherit()));
  board.styleMap.add("item_separator", termui::Style(offsetColor(60), termui::Color::Inherit()));

  renderer = tui::Board();
}

void KAN::init() {
  /*
  creates directories, and files to prepopulate the system
  */

  // create some directories
  std::filesystem::create_directory(_path);
  std::filesystem::create_directory(_path / "jobs");

  // board
  kan::Board board = Board(_path);
  board.title      = "New Project";

  board.columns.push_back(Column(YAML::Load(R"({"title": "Backlog",})")));

  board.columns.push_back(Column(YAML::Load(R"({"title": "Development",})")));

  board.columns.push_back(Column(YAML::Load(R"({"title": "Review",})")));

  board.columns.push_back(Column(YAML::Load(R"({"title": "Done",})")));

  std::ofstream out_file(_path / "kan.yaml");
  out_file << board.toYaml();

  Timestamp timestamp;

  kan::Job first_job;
  first_job.title       = "First Job";
  first_job.description = "Welcome to KAN!";
  first_job.status      = "Backlog";

  first_job.history.addEvent({"Job created.", User().getFromSystem(), Timestamp().Now()});

  std::ofstream out_file_job(_path / "jobs" / std::format("{}.yaml", first_job.uuid));
  out_file_job << first_job.toYaml();

  // contributors - filled in as people open the board (see tui::Board::show)
  board.saveContributors();

  board.styleMap.add("homepage_title", termui::Styles::bold);
  board.saveStyleMap();
};

void KAN::start() { renderer.show(); }

} // namespace kan
