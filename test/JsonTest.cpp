#include <gtest/gtest.h>

#include "Json.h"

using namespace jerry;
TEST(JsonParseTest, JsonTestInit) {
  auto json = Json::fromString("hello");
  ASSERT_TRUE(json);
  EXPECT_EQ(json->getValue(), JsonValue{"hello"});
}

// TEST(JsonParseTest, JsonSimpleKeyValueTest) {
//   auto json = Json::fromString("\"key\" : \"value\"");
//   ASSERT_TRUE(json);
//   EXPECT_EQ(json.value["key"], JsonValue{"value"});
//   // EXPECT_EQ(json->getValue(), JsonValue{std::map<std::string, JsonValue>{"hello"});
// }
