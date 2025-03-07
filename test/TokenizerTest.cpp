#include <gtest/gtest.h>
#include "Tokenizer.h"
using namespace jerry;

TEST(TokenizerTest, TokenizerExampleTest) {
  // Good example of creating a Tokenizer.
  // Creates a Tokenizer<char> with the lambda passed in.
  // The lambda attempts to read the char and if it is at the current position then advance().
  // Otherwise we do the monad thing and return nullopt.
	auto f = Tokenizer<char>([=](TokenizerState state) -> std::optional<std::pair<char, TokenizerState>> {
      char expected = 't';
			if (state.currentCharacter() == expected) {
					return std::make_pair(expected, state.advance());
			}
			return std::nullopt;
	});

	// State carries the position and the input string.
  TokenizerState state = TokenizerState::init("the quick brown fox jumped over the fella", 0);
  Tokenizer toke = Tokenizer(f);
  // When run is executed on a Tokenizer. We attempt to do the monad's operator.
  // This may return a nullopt if unsuccesful.
  // If succesful return the value and the new state.
	
  EXPECT_EQ(state.currentCharacter(), 't');
  std::optional<std::pair<char, TokenizerState>> newState = toke.run(state);
  EXPECT_EQ(newState->second.currentCharacter(), 'h');
}
