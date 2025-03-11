#pragma once
#include <cstddef>
#include <optional>
#include <string>
#include <functional>
#include <cassert>
#include <vector>

#include "JsonToken.h"

namespace jerry {
  class TokenizerState {
  private:
    std::string input;
    size_t position;
  public:
    explicit TokenizerState(std::string s, size_t pos);
    static TokenizerState init(std::string s, size_t pos);
    char currentCharacter() const noexcept;
    size_t getPosition() const noexcept;
    std::string getInputString() const noexcept{
      return input;
    }
    size_t getInputStringSize() const noexcept {
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
    /**
     * Bind is a mutation that uses f(T) to give a Tokenizer<U>.
     * This allows for chaining operations together.
     */
    template<typename U>
    Tokenizer<U> bind(std::function<Tokenizer<U>(T)> f) const {
      TokenizerFunc currentFunc = func;
      auto transform = f;
      return Tokenizer<U>([currentFunc = std::move(currentFunc), 
                          transform = std::move(transform)](TokenizerState state) -> std::optional<std::pair<U, TokenizerState>> {
          auto result = currentFunc(state);
          if (!result) {
            return std::nullopt;
          }
          
          T val = result->first;
          TokenizerState newState = result->second;

          Tokenizer<U> newTokenizer = transform(val);
          return newTokenizer.run(newState);
         });
    }

    /** 
     * Map is basically a transormation via f(T) -> U
     * It uses the calling tokenizer run() to grab it's token,
     * then it runs f on the result, giving U. No state is 
     * changed. This could be useful for getting the lower case
     * value of a char for example.
     * in other words: f(T)--> Tokenizer<U> 
     */
    template<typename U>
    Tokenizer<U> map(std::function<U(T)> f) const {
      return Tokenizer<U>([currentFunc = std::move(func),
        transform = std::move(f), this](TokenizerState state) -> std::optional<std::pair<U, TokenizerState>> {
          auto result = currentFunc(state);
          if (!result) {
            return std::nullopt;
          }
          U transformation = transform(result->first);
          return std::make_pair(transformation, result->second);
         });
    }
  };

  /** 
   * Tries to run x. If x returns nullopt run y.
   */
  template<typename T>
  static Tokenizer<T> orElse(Tokenizer<T> x, Tokenizer<T> y) {
    return Tokenizer<T>([=](TokenizerState state) -> std::optional<std::pair<T, TokenizerState>> {
      auto r = x.run(state);
      if (r) {
        return std::make_pair(r->first, r->second);
      }
      /**
       * Cool shit: By atomically updating state we can basically try one operation then "roll back"
       *   the original state if things blow up.
       */
      r = y.run(state);
      if (r) {
        return std::make_pair(r->first, r->second);
      }
      return std::nullopt;
    });
  }

  template<typename T>
  static Tokenizer<std::vector<T>> manyOf(Tokenizer<T> x) {
    return Tokenizer<std::vector<T>>([x](TokenizerState state) -> std::optional<std::pair<std::vector<T>, TokenizerState>> {
      std::vector<T> gotTokens;
      while(true) {
        auto r = x.run(state);
        if (!r || state.getPosition() >= state.getInputStringSize()) {
          break;
        }
        gotTokens.push_back(r->first);
        state = r->second;
      }
      return std::make_pair(gotTokens, state);
    });
  }

  // Generators for different token types
	/** Returns the same state **/
  template<typename T>
  static Tokenizer<T> pure(T value) {
    return Tokenizer<T>([=](TokenizerState state) -> std::optional<std::pair<T, TokenizerState>> {
        return std::make_pair(value, state);
      }
    );
  }

	/** Returns the same state **/
  template<typename T>
  static Tokenizer<T> match(T value, std::function<bool(T)> matcher) {
    return Tokenizer<T>([matcher = std::move(matcher), value](TokenizerState state) -> std::optional<std::pair<T, TokenizerState>> {
      if (matcher(value)) {
        return pure<T>(value).run(state);
      }
      return std::nullopt;
      }
    );
  }

  template<typename T>
  static Tokenizer<T> isEqual(T value, T other) {
    auto equalityChecker = [other](T val) {
      return val == other;
    };
    return match<T>(value, equalityChecker);
  }

  template<typename T>
  static Tokenizer<T> isNotEqual(T value, T other) {
    auto equalityChecker = [other](T val) {
      return val != other;
    };
    return match<T>(value, equalityChecker);
  }

  static Tokenizer<char> isDigit(char c) {
    auto digitChecker = [](char c) {
      return c >= '0' && c <= '9';
    };
    return match<char>(c, digitChecker);
  }

	/** Returns the same state **/
  template<typename T>
  static Tokenizer<T> fail() {
    return Tokenizer<T>([=](TokenizerState) -> std::optional<std::pair<T, TokenizerState>> {
        return std::nullopt;
        }
    );
  }

  static Tokenizer<char> character() {
    return Tokenizer<char>([](TokenizerState state) -> std::optional<std::pair<char, TokenizerState>> {
        if (state.getPosition() >= state.getInputStringSize()) {
          return std::nullopt;
        }
				char val = state.currentCharacter();
        return std::make_pair(val, state.advance());
        }
    );
  }

  static Tokenizer<uint> digit() {
    auto asDigit = [](char c) {
      return static_cast<uint>(c - '0');
    };
    
    return character().bind<uint>([asDigit](char c) {
      return isDigit(c).map<uint>(asDigit);
    });
  }

  static Tokenizer<char> whitespace() {
    return character().bind<char>([](char c) {
      return isEqual(c, ' ');
    });
  }

  [[maybe_unused]]
  static Tokenizer<char> braceOpen() {
    return character().bind<char>([](char c) {
      return isEqual(c, '{');
    });
  }

  [[maybe_unused]]
  static Tokenizer<char> braceClose() {
    return character().bind<char>([](char c) {
      return isEqual(c, '}');
    });
  }

  [[maybe_unused]]
  static Tokenizer<char> bracketOpen() {
    return character().bind<char>([](char c) {
      return isEqual(c, '[');
    });
  }

  [[maybe_unused]]
  static Tokenizer<char> bracketClose() {
    return character().bind<char>([](char c) {
      return isEqual(c, ']');
    });
  }

  [[maybe_unused]]
  static Tokenizer<char> colon() {
    return character().bind<char>([](char c) {
      return isEqual(c, ':');
    });
  }

  [[maybe_unused]]
  static Tokenizer<char> comma() {
    return character().bind<char>([](char c) {
      return isEqual(c, ',');
    });
  }

  [[maybe_unused]]
  static Tokenizer<char> doubleQuote() {
    return character().bind<char>([](char c) {
      return isEqual(c, '"');
    });
  }

  static Tokenizer<std::string> word() {
    return manyOf<char>(character().bind<char>([](char c){
      return (c == ' ' || c == '\t' || c == '\n') ? fail<char>() : pure(c);
    })).bind<std::string>([](std::vector<char> chars){
      return pure(std::string(chars.begin(), chars.end()));
    });
  }


  // Consumes word+whitespace sequences until position >= input.size()
  [[maybe_unused]]
  static Tokenizer<std::vector<std::string>> sentence() {
    auto wordFollowedBySpace = word().bind<std::string>([](std::string w) {
      return manyOf<char>(whitespace()).map<std::string>([w](std::vector<char>) {
        return w;
      });
    });
    return manyOf<std::string>(wordFollowedBySpace);
  }

  [[maybe_unused]]
  static Tokenizer<JsonToken> objectStart() {
    return character().bind<JsonToken>([](char c) {
      return isEqual(c, '{').bind<JsonToken>([](char c){
        return pure(JsonToken::makeStructural(JsonTokenType::ObjectStart));
      });
    });
  }

  [[maybe_unused]]
  static Tokenizer<JsonToken> objectEnd() {
    return character().bind<JsonToken>([](char c) {
      return isEqual(c, '}').bind<JsonToken>([](char c){
        return pure(JsonToken::makeStructural(JsonTokenType::ObjectEnd));
      });
    });
  }

  [[maybe_unused]]
  static Tokenizer<JsonToken> arrayStart() {
    return character().bind<JsonToken>([](char c) {
      return isEqual(c, '[').bind<JsonToken>([](char c){
        return pure(JsonToken::makeStructural(JsonTokenType::ArrayStart));
      });
    });
  }

  [[maybe_unused]]
  static Tokenizer<JsonToken> arrayEnd() {
    return character().bind<JsonToken>([](char c) {
      return isEqual(c, ']').bind<JsonToken>([](char c){
        return pure(JsonToken::makeStructural(JsonTokenType::ArrayEnd));
      });
    });
  }

  [[maybe_unused]]
  static Tokenizer<JsonToken> jsonString() {
    return doubleQuote().bind<std::string>([](char) {
      return manyOf<char>(character().bind<char>([](char c){
        return isNotEqual<char>(c, '"');
      })).bind<std::string>([](std::vector<char> chars){
        // Create a string from the characters
        std::string s(chars.begin(), chars.end());
        
        // Consume closing quote
        return doubleQuote().map<std::string>([s](char) {
          return s;
        });
      });
    }).map<JsonToken>([](std::string s) {
      // Convert string to JsonToken
      return JsonToken::fromString(s);
    });
  }
}