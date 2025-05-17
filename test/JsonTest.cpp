#include <gtest/gtest.h>

#include "Json.h"

using namespace jerry;
TEST(JsonParseTest, JsonTestStringLiteral) {
  std::string input = "\"hello\"";
  auto json = Json::fromString(input);
  ASSERT_TRUE(json);
  
  auto jsonValue = json->getValue();
  auto strOpt = jsonValue.toString();
  ASSERT_TRUE(strOpt.has_value());
  EXPECT_EQ(strOpt.value(), "hello");
}

TEST(JsonParseTest, JsonTestList) {
  std::string input = "[\"hello\", \"beautiful\", \"world\"]";
  auto expected = JsonValue({"hello", "beautiful", "world"});
  auto json = Json::fromString(input);
  ASSERT_TRUE(json);
  EXPECT_EQ(json->getValue(), expected);
}

class JsonParseTest
    : public ::testing::TestWithParam<std::pair<std::string, std::unordered_map<std::string, JsonValue>>> {};

TEST_P(JsonParseTest, JsonObjectParseTest) {
  const auto& [input, expected] = GetParam();
  auto json = Json::fromString(input);
  ASSERT_TRUE(json);

  auto jsonValue = json->getValue();
  EXPECT_EQ(jsonValue, expected);
}

INSTANTIATE_TEST_SUITE_P(
    JsonParseTests, JsonParseTest,
    ::testing::Values(
        std::make_pair("{\"hey\" : \"dude\"", std::unordered_map<std::string, JsonValue>{{"hey", JsonValue("dude")}}),
        std::make_pair("{\"enable gamer mode?\" : true", std::unordered_map<std::string, JsonValue>{{"enable gamer mode?", JsonValue(true)}})
      ));