#include "helpers.hpp"

using namespace testing_kan;

TEST(styling, ParsesAndNamesColours) {
  using kan::styling::colorName;
  using kan::styling::parseColor;

  EXPECT_EQ(parseColor("default"), termui::Color::Inherit());
  EXPECT_EQ(parseColor(""), termui::Color::Inherit());
  EXPECT_EQ(parseColor("Red"), termui::Color(uint8_t(1)));
  EXPECT_EQ(parseColor("grey"), termui::Color(uint8_t(8)));
  EXPECT_EQ(parseColor("208"), termui::Color(uint8_t(208)));
  EXPECT_EQ(parseColor("#FF8800"), termui::Color(uint8_t(255), uint8_t(136), uint8_t(0)));
  EXPECT_EQ(parseColor("256"), std::nullopt);
  EXPECT_EQ(parseColor("#ff88"), std::nullopt);
  EXPECT_EQ(parseColor("#gg0000"), std::nullopt);
  EXPECT_EQ(parseColor("reddish"), std::nullopt);

  for (std::string name : {"default", "black", "bright-cyan", "208", "#ff8800"}) {
    EXPECT_EQ(colorName(*parseColor(name)), name);
  }
}

TEST(styling, ColumnHeaderRoundTrips) {
  kan::Column column;
  column.title  = "Review";
  column.header = termui::Style(termui::Color(uint8_t(3)), termui::Color("#202020"), {termui::SGR::Italic, termui::SGR::Underlined});

  auto yaml = column.toYaml();
  EXPECT_EQ(yaml["header"]["foreground"].as<std::string>(), "yellow");
  EXPECT_EQ(yaml["header"]["background"].as<std::string>(), "#202020");

  EXPECT_EQ(kan::Column(yaml).header, column.header);

  kan::Column plain;
  plain.title = "Backlog";
  EXPECT_FALSE(plain.toYaml()["header"]); // the default isn't written out
  EXPECT_EQ(kan::Column(plain.toYaml()).header, kan::Column::defaultHeader());
}
