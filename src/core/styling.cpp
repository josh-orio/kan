#include <kan/core.hpp>

#include <algorithm>
#include <format>

namespace kan {
namespace styling {

// the 16 standard terminal colours, in their ansi order
static const std::vector<std::string> names = {"black",       "red",          "green",      "yellow",       "blue",          "magenta",     "cyan",
                                               "white",       "bright-black", "bright-red", "bright-green", "bright-yellow", "bright-blue", "bright-magenta",
                                               "bright-cyan", "bright-white"};

static std::string lower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
  return s;
}

std::optional<termui::Color> parseColor(std::string text) {
  text = lower(text);
  text.erase(0, text.find_first_not_of(" "));
  text.erase(text.find_last_not_of(" ") + 1);

  if (text.empty() || text == "default" || text == "none") {
    return termui::Color::Inherit();
  }

  if (text == "gray" || text == "grey") {
    text = "bright-black";
  }

  if (auto it = std::find(names.begin(), names.end(), text); it != names.end()) {
    return termui::Color(static_cast<uint8_t>(it - names.begin()));
  }

  if (text.find_first_not_of("0123456789") == std::string::npos && text.size() <= 3) {
    auto n = std::stoi(text);
    return n <= 255 ? std::optional(termui::Color(static_cast<uint8_t>(n))) : std::nullopt;
  }

  if (text.size() == 7 && text[0] == '#' && text.find_first_not_of("0123456789abcdef", 1) == std::string::npos) {
    return termui::Color(text);
  }

  return std::nullopt;
}

std::string colorName(const termui::Color &color) {
  switch (color.mode) {
  case termui::Color::Mode::Ansi256:
    return color.value < names.size() ? names[color.value] : std::to_string(color.value);
  case termui::Color::Mode::TrueColor:
    return std::format("#{:02x}{:02x}{:02x}", color.rgb.r, color.rgb.g, color.rgb.b);
  default:
    return "default";
  }
}

const std::vector<termui::Color> &palette() {
  static std::vector<termui::Color> colors = [] {
    std::vector<termui::Color> out = {termui::Color::Inherit()};
    for (uint8_t i = 0; i < 16; i++) {
      out.push_back(termui::Color(i));
    }
    return out;
  }();

  return colors;
}

const std::vector<std::pair<termui::SGR, std::string>> &attributes() {
  static std::vector<std::pair<termui::SGR, std::string>> list = {
      {termui::SGR::Bold, "bold"},
      {termui::SGR::Faint, "faint"},
      {termui::SGR::Italic, "italic"},
      {termui::SGR::Underlined, "underlined"},
      {termui::SGR::ReverseVideo, "reverse"},
      {termui::SGR::CrossedOut, "crossed-out"},
  };

  return list;
}

YAML::Node toYaml(const termui::Style &style) {
  YAML::Node yaml;
  yaml["foreground"] = colorName(style.foregroundColor);
  yaml["background"] = colorName(style.backgroundColor);

  YAML::Node attrs(YAML::NodeType::Sequence);
  attrs.SetStyle(YAML::EmitterStyle::Flow); // attributes: [bold, italic]
  for (auto &[sgr, name] : attributes()) {
    if (std::find(style.attributes.begin(), style.attributes.end(), sgr) != style.attributes.end()) {
      attrs.push_back(name);
    }
  }
  yaml["attributes"] = attrs;

  return yaml;
}

termui::Style fromYaml(YAML::Node yaml, termui::Style fallback) {
  if (!yaml || !yaml.IsMap()) {
    return fallback;
  }

  auto style = fallback;

  if (auto fg = parseColor(yaml["foreground"].as<std::string>("default"))) {
    style.foregroundColor = *fg;
  }
  if (auto bg = parseColor(yaml["background"].as<std::string>("default"))) {
    style.backgroundColor = *bg;
  }

  if (auto attrs = yaml["attributes"]; attrs && attrs.IsSequence()) {
    style.attributes.clear();

    for (const auto &a : attrs) {
      auto name = lower(a.as<std::string>(""));
      for (auto &[sgr, n] : attributes()) {
        if (n == name) {
          style.attributes.push_back(sgr);
        }
      }
    }
  }

  return style;
}

} // namespace styling
} // namespace kan
