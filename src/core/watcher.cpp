#include <kan/core.hpp>

#include <functional>

namespace kan {

static size_t mix(size_t h, size_t v) { return h ^ (v + 0x9e3779b97f4a7c15 + (h << 6) + (h >> 2)); }

static size_t stamp(const std::filesystem::path &path) {
  std::error_code ec;

  auto time = std::filesystem::last_write_time(path, ec);
  if (ec) {
    return 1; // missing
  }

  auto size = std::filesystem::is_regular_file(path, ec) ? std::filesystem::file_size(path, ec) : 0;

  return mix(std::hash<std::string>{}(path.string()), mix(time.time_since_epoch().count(), size));
}

Watcher::Watcher(std::vector<std::filesystem::path> paths) : _paths(paths), _signature(signature()) {}

size_t Watcher::signature() const {
  size_t h = 0;

  for (const auto &path : _paths) {
    h = mix(h, stamp(path));

    std::error_code ec;
    if (!std::filesystem::is_directory(path, ec)) {
      continue;
    }

    size_t entries = 0; // summed so the order the directory lists them in doesn't matter
    for (const auto &entry : std::filesystem::directory_iterator(path, ec)) {
      entries += stamp(entry.path());
    }

    h = mix(h, entries);
  }

  return h;
}

bool Watcher::changed() {
  auto now = signature();

  if (now == _signature) {
    return false;
  }

  _signature = now;
  return true;
}

void Watcher::reset() { _signature = 0; }

} // namespace kan
