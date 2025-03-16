#include <gtest/gtest.h>

#include "Json.h"

using namespace jerry;
TEST(JsonParseTest, JsonTestInit) {
  auto json = Json::fromString("hello");
  ASSERT_TRUE(json);
  EXPECT_EQ(json->getValue(), JsonValue{"hello"});
}
