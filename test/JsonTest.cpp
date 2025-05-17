#include <gtest/gtest.h>

#include "Json.h"

using namespace jerry;

class JsonParseTest
    : public ::testing::TestWithParam<std::pair<std::string, Json>> {};

TEST_P(JsonParseTest, jsonObjectParseTest) {
  const auto& [input, expected] = GetParam();
  auto json = Json::fromString(input);
  ASSERT_TRUE(json);

  EXPECT_EQ(json, expected);
}

INSTANTIATE_TEST_SUITE_P(
    JsonParseTests, JsonParseTest,
    ::testing::Values(
        std::make_pair("[\"hello\", \"beautiful\", \"world\"]", Json(std::vector<JsonValue>({"hello", "beautiful", "world"}))),
        std::make_pair("{\"hey\" : \"dude\"", Json(JsonValue(std::unordered_map<std::string, JsonValue>{{"hey", JsonValue("dude")}}))),
        std::make_pair("true", Json(JsonValue(true))),
        std::make_pair("{\"enable gamer mode?\" : true", Json(JsonValue(std::unordered_map<std::string, JsonValue>{{"enable gamer mode?", JsonValue(true)}})))
      ));