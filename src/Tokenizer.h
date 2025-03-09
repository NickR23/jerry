#pragma once
#include <cstddef>
#include <optional>
#include <string>
#include <functional>
#include <cassert>

namespace jerry {
  class TokenizerState {
  private:
    std::string input;
    size_t position;
  public:
    explicit TokenizerState(std::string s, size_t pos);
    static TokenizerState init(std::string s, size_t pos);
    char currentCharacter() const;
    size_t getPosition() const;
    std::string getInputString() const {
      return input;
    }
    size_t getInputStringSize() const {
      return input.size();
    }
    TokenizerState advance() const;
  };

  template<typename T>
  class Tokenizer {
  private:
    // Used during bind operations. 
    using TokenizerFunc = std::function<std::optional<std::pair<T, TokenizerState>>(TokenizerState)>;
    TokenizerFunc func;
  public:
    explicit Tokenizer(TokenizerFunc f): func(f) {};
    static Tokenizer<T> init(TokenizerFunc f) {
      Tokenizer<T> toke = Tokenizer(f);
      return toke;
    }

    std::optional<std::pair<T, TokenizerState>> run(TokenizerState s) const {
      // The TokenizerFunc passes it's execution result back up the stack.
      // TokenizerFunc == std::optional<std::pair<T, TokenizerState>>
			assert(this->func != nullptr);
      return this->func(s);
    }

    // Used to sequence operations.
    template<typename U>
    Tokenizer<U> bind(std::function<Tokenizer<U>(T)> f) const {
			assert(f != nullptr); // Add this check
      return Tokenizer<U>([=, this](TokenizerState state) -> std::optional<std::pair<U, TokenizerState>> {
          // Run this Tokenizer
          auto result = this->run(state);
          // If this tokenizer fails then return nullopt
          if (!result) {
            return std::nullopt;
          }
          
          // Extract return values from this Tokenizer
          T val = result->first;
          TokenizerState newState = result->second;

          // Grab the new tokenizer given the value of the current.
          Tokenizer<U> newTokenizer = f(val);
          // Return the result of the bound Tokenizer.
          return newTokenizer.run(newState);
         });
    }
  };

  // Generators for different token types

	/** Returns the same state **/
  static Tokenizer<char> pure() {
    return Tokenizer<char>([=](TokenizerState state) -> std::optional<std::pair<char, TokenizerState>> {
        return std::make_pair(state.currentCharacter(), state);
        }
    );
  }
  static Tokenizer<char> character() {
    return Tokenizer<char>([=](TokenizerState state) -> std::optional<std::pair<char, TokenizerState>> {
        if (state.getPosition() >= state.getInputStringSize()) {
          return std::nullopt;
        }
				char val = state.currentCharacter();
        return std::make_pair(val, state.advance());
        }
    );
  }

  static Tokenizer<std::string> word() {
    return Tokenizer<std::string>([=](TokenizerState state) -> std::optional<std::pair<std::string, TokenizerState>> {
        if (state.getPosition() >= state.getInputStringSize()) {
          return std::nullopt;
        }
				std::string word;
				while (state.getPosition() < state.getInputStringSize() && 
							state.currentCharacter() != ' ') {
					char val = state.currentCharacter();
					word.push_back(val);
					state = state.advance();
				}
        return std::make_pair(word, state);
        }
    );
  }

  static Tokenizer<char> whitespace() {
    return Tokenizer<char>([=](TokenizerState state) -> std::optional<std::pair<char, TokenizerState>> {
        if (state.getPosition() >= state.getInputStringSize() ||
						state.currentCharacter() != ' ') {
          return std::nullopt;
        }
				char val = state.currentCharacter();
        return std::make_pair(val, state.advance());
        }
    );
  }

}
