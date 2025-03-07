#include <gtest/gtest.h>
#include "Tokenizer.h"

class TokenizerStateTest :public ::testing::TestWithParam<std::tuple<std::string, int, char>> {};

TEST_P(TokenizerStateTest, TokenStateInit) {
  const auto& [input, position, expected_char] = GetParam();
  std::optional<jerry::TokenizerState> state = jerry::TokenizerState::init(input, position);
  ASSERT_TRUE(state);
  EXPECT_EQ(state->currentCharacter(), expected_char);
}
INSTANTIATE_TEST_SUITE_P(
    GetPositionTests,
    TokenizerStateTest,
    ::testing::Values(
      std::make_tuple("{\"message\": \"hello world\"}", 0, '{'),
      std::make_tuple("{\"message\": \"hello world\"}", 1, '"'),
      std::make_tuple("{\"message\": \"hello world\"}", 2, 'm'),
      std::make_tuple("{\"message\": \"hello world\"}", 12, '"'),
      std::make_tuple("{\"message\": \"hello world\"}", 25, '}')
    )
);

TEST(TokenizerStateTest, TokenStateInitInvalid) {
  // Initializing position past input string size.
  std::string input = "{\"message\": \"hello world\"}";
  size_t position = input.size();
  std::optional<jerry::TokenizerState> state = jerry::TokenizerState::init(input, position);
  ASSERT_FALSE(state);
}
