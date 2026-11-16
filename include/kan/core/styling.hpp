#ifndef CORE_STYLING_HPP
#define CORE_STYLING_HPP

#include <optional>
#include <string>
#include <termui/termui.hpp>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace kan {
namespace styling {

// colours are written the way people would type them: default, a name (red, bright-blue), 0-255 or #rrggbb
std::optional<termui::Color> parseColor(std::string text);
std::string                  colorName(const termui::Color &color);

const std::vector<termui::Color> &palette(); // default then the 16 named terminal colours, what ←→ steps through

// the attributes a column header can use, with their names in kan.yaml
const std::vector<std::pair<termui::SGR, std::string>> &attributes();

YAML::Node    toYaml(const termui::Style &style);
termui::Style fromYaml(YAML::Node yaml, termui::Style fallback); // anything missing or unreadable keeps the fallback's value

} // namespace styling
} // namespace kan

#endif
