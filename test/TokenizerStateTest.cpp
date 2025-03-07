#include <gtest/gtest.h>
#include "Tokenizer.h"

class TokenizerStateTest :public ::testing::TestWithParam<std::tuple<std::string, int, char>> {};

TEST_P(TokenizerStateTest, TokenizerStateInit) {
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

TEST(TokenizerStateTest, TokenizerStateInitInvalid) {
  // Initializing position past input string size.
  std::string input = "{\"message\": \"hello world\"}";
  size_t position = input.size();
  std::optional<jerry::TokenizerState> state = jerry::TokenizerState::init(input, position);
  ASSERT_FALSE(state);
}

TEST_P(TokenizerStateTest, TokenizerStateAdvanceTest) {
  const auto& [input, iterations, expected_char] = GetParam();
  std::optional<jerry::TokenizerState> state = jerry::TokenizerState::init(input, 0);
  for (int i = 0; i < iterations; i++) {
    state = state->advance();
  }
  ASSERT_TRUE(state);
  EXPECT_EQ(state->currentCharacter(), expected_char);
}

INSTANTIATE_TEST_SUITE_P(
    AdvancePositionsTest,
    TokenizerStateTest,
    ::testing::Values(
      std::make_tuple("{\"message\": \"hello world\"}", 0, '{'),
      std::make_tuple("{\"message\": \"hello world\"}", 1, '"'),
      std::make_tuple("{\"message\": \"hello world\"}", 2, 'm'),
      std::make_tuple("{\"message\": \"hello world\"}", 12, '"'),
      std::make_tuple("{\"message\": \"hello world\"}", 25, '}')
    )
);

TEST(TokenizerStateTest, TokenizerStateAdvanceInvalidTest) {
  std::string input = "{\"message\": \"hello world\"}";
  size_t position = input.size();
  std::optional<jerry::TokenizerState> state = jerry::TokenizerState::init(input, 0);
  for (int i = 0; i < input.size() - 1; i++) {
    state = state->advance();
  }
  ASSERT_TRUE(state);
  EXPECT_EQ(state->currentCharacter(), '}');
  EXPECT_EQ(state->getPosition(), input.size() - 1);

  // When I advance past the end of the input string, this should be nullopt.
  state = state->advance();
  ASSERT_TRUE(!state);
}
