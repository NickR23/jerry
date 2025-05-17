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

// TODO Parameterize this and add more tests...
TEST(JsonParseTest, JsonTestObject) {
  std::string input = "{\"hey\" : \"dude\"";
  auto expected = std::unordered_map<std::string, JsonValue>({
    {"hey", JsonValue("dude")}
  });
  auto json = Json::fromString(input);
  ASSERT_TRUE(json);
  
  auto jsonValue = json->getValue();
  EXPECT_EQ(jsonValue, expected);
}
