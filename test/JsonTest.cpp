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
    std::make_pair("{\"hey\" : \"dude\"}", Json(JsonValue(std::unordered_map<std::string, JsonValue>{{"hey", JsonValue("dude")}}))),
    std::make_pair("true", Json(JsonValue(true))),
    std::make_pair("{\"enable gamer mode?\" : true}", Json(JsonValue(std::unordered_map<std::string, JsonValue>{{"enable gamer mode?", JsonValue(true)}}))),
    std::make_pair("{\"NESTED json objects\" : {\"cowabunga\" : [\"surfs\",\"up\"]}}",
      Json(JsonValue(std::unordered_map<std::string, JsonValue>{
      {"NESTED json objects",
       JsonValue(std::unordered_map<std::string, JsonValue>{
         {"cowabunga", JsonValue(std::vector<JsonValue>({"surfs", "up"}))}
       })
      }
      }))
    ),
    std::make_pair("123", Json(JsonValue(123))),
    std::make_pair("-45", Json(JsonValue(-45))),
    // TODO parsing doubles is broken (rounding errors)
    std::make_pair("-45.67", Json(JsonValue(-45.67))),
    std::make_pair("null", Json(JsonValue())),
    std::make_pair("false", Json(JsonValue(false))),
    std::make_pair("[1, 2, 3, 4]", Json(std::vector<JsonValue>({1, 2, 3, 4}))),
    std::make_pair("{\"a\":1,\"b\":2}", Json(JsonValue(std::unordered_map<std::string, JsonValue>{{"a", 1}, {"b", 2}}))),
    std::make_pair("{\"emptyArray\":[]}", Json(JsonValue(std::unordered_map<std::string, JsonValue>{{"emptyArray", JsonValue(std::vector<JsonValue>{})}}))),
    std::make_pair("{\"emptyObject\":{}}", Json(JsonValue(std::unordered_map<std::string, JsonValue>{{"emptyObject", JsonValue(std::unordered_map<std::string, JsonValue>{})}}))),
    std::make_pair("[{\"x\":1}, {\"y\":2}]", Json(std::vector<JsonValue>{
      JsonValue(std::unordered_map<std::string, JsonValue>{{"x", 1}}),
      JsonValue(std::unordered_map<std::string, JsonValue>{{"y", 2}})
    })),
    std::make_pair("{\"nested\":[{\"a\":true}, {\"b\":false}]}", Json(JsonValue(std::unordered_map<std::string, JsonValue>{
      {"nested", JsonValue(std::vector<JsonValue>{
        JsonValue(std::unordered_map<std::string, JsonValue>{{"a", true}}),
        JsonValue(std::unordered_map<std::string, JsonValue>{{"b", false}})
      })}
    }))),
    std::make_pair("\"string with \\\"escaped quotes\\\"\"", Json(JsonValue("string with \"escaped quotes\""))),
    std::make_pair("{\"unicode\":\"\\u263A\"}", Json(JsonValue(std::unordered_map<std::string, JsonValue>{{"unicode", JsonValue("\u263A")}})))
  )
);