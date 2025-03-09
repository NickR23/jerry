#include <gtest/gtest.h>
#include "Tokenizer.h"
#include <iostream>
using namespace jerry;

TEST(TokenizerTest, TokenizerExampleTest) {
  // Good example of creating a Tokenizer.
  // Creates a Tokenizer<char> with the lambda passed in.
  // The lambda attempts to read the char and if it is at the current position then advance().
  // Otherwise we do the monad thing and return nullopt.
  auto toke = character();

	// State carries the position and the input string.
  TokenizerState state = TokenizerState::init("the quick brown fox jumped over the fella", 0);
  // When run is executed on a Tokenizer. We attempt to do the monad's operator.
  // This may return a nullopt if unsuccesful.
  // If succesful return the value and the new state.
	
  EXPECT_EQ(state.currentCharacter(), 't');
  std::optional<std::pair<char, TokenizerState>> newState = toke.run(state);
  EXPECT_EQ(newState->second.currentCharacter(), 'h');
}

TEST(TokenizerTest, PureTest) {
  std::string input = "the quick brown fox";
  auto pureToken = pure('t');
  auto result = pureToken.run(TokenizerState::init(input, 0));
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ('t', result->first);
}

TEST(TokenizerTest, ParseWordTest) {
  std::string input = "the quick brown fox";
  auto wordTokenizer = word();
  auto result = wordTokenizer.run(TokenizerState::init(input, 0));
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ("the", result->first);
  EXPECT_EQ(3, result->second.getPosition());
}

TEST(TokenizerTest, ParseSentenceBindTest) { 
  std::string input = "the quick brown fox";
  std::vector<std::string> expectedTokens = {
    "the",
    "quick",
    "brown",
    "fox"
  };
  // First, parse the first word directly
  auto wordTokenizer = word();
  auto currentState = TokenizerState::init(input, 0);
  auto r = sentence().run(currentState);

  ASSERT_TRUE(r);
  EXPECT_EQ(expectedTokens, r->first);
  EXPECT_EQ(r->second.getPosition(), input.size());
}

TEST(TokenizerTest, MapCharTest) { 
  std::string input = "the quick brown fox";
  auto state = TokenizerState::init(input, 0);
  auto toUpper = character().map<char>([=](char c) {
    return std::toupper(c);
  });

  auto r = toUpper.run(state);
  ASSERT_TRUE(r);
  EXPECT_EQ('T', r->first);
  EXPECT_EQ(r->second.getPosition(), 1);
}

TEST(TokenizerTest, braceOpenTest) { 
  std::string input = "{{}}";
  auto state = TokenizerState::init(input, 0);
  int i = 0;
  while(true) {
    auto r = braceOpen().run(state);
    i++;
    if (!r) {
      break;
    }
    state = r->second;
    EXPECT_EQ('{', r->first);
    EXPECT_EQ(state.getPosition(), i);
  }
  EXPECT_EQ(state.getPosition(), 2);
}