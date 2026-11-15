#ifndef CORE_WATCHER_HPP
#define CORE_WATCHER_HPP

#include <filesystem>
#include <vector>

namespace kan {

// notices when files change by checking their modification times and sizes, so it works the same on every OS.
// a directory counts as changed when anything directly inside it is added, removed or rewritten
class Watcher {
private:
  std::vector<std::filesystem::path> _paths;
  size_t                             _signature = 0;

  size_t signature() const;

public:
  Watcher() = default;
  Watcher(std::vector<std::filesystem::path> paths);

  bool changed(); // since the last call, or since the watcher was made
  void reset();   // the next changed() says yes, eg to try again after reading a half written file
};

} // namespace kan

#endif
