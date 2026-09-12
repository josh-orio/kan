#include <kan/core.hpp>

namespace kan {

// ---- Issue ----
Issue::Issue() {}

Issue::Issue(nlohmann::json json) { fromJson(json); }

void Issue::fromJson(nlohmann::json Issue) {}

nlohmann::json Issue::toJson() {}

// ---- Issues ----
Issues::Issues() {}

Issues::Issues(nlohmann::json json) { fromJson(json); }

void Issues::fromJson(nlohmann::json Issue) {}

nlohmann::json Issues::toJson() {}

size_t Issues::size() { return _issues.size(); }

Issue Issues::at(int i) { return _issues.at(i); }

} // namespace kan
