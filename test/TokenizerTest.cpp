#include <gtest/gtest.h>
#include "Tokenizer.h"

class TokenizerStateTest :public ::testing::TestWithParam<std::tuple<std::string, int, char>> {};


TEST_P(TokenizerStateTest, SanityTest) {
  const auto& [input, position, expected_char] = GetParam();
  jerry::TokenizerState state = jerry::TokenizerState(input, position);
  EXPECT_EQ(state.currentCharacter(), expected_char);
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
