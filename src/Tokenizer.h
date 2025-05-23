#pragma once
#include <cassert>
#include <cstddef>
#include <functional>
#include <optional>
#include <string>
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
  std::string getInputString() const noexcept { return input; }
  size_t getInputStringSize() const noexcept { return input.size(); }
  TokenizerState advance() const;
};

template <typename T> class Tokenizer {
private:
  // Used during bind operations.
  using TokenizerFunc =
      std::function<std::optional<std::pair<T, TokenizerState>>(
          TokenizerState)>;
  TokenizerFunc func;

public:
  explicit Tokenizer(TokenizerFunc f) : func(f) {
    assert(this->func != nullptr);
  };

  static Tokenizer<T> init(TokenizerFunc f) {
    Tokenizer<T> toke = Tokenizer(f);
    return toke;
  }

  std::optional<std::pair<T, TokenizerState>> run(TokenizerState s) const {
    // The TokenizerFunc passes it's execution result back up the stack.
    // TokenizerFunc == std::optional<std::pair<T, TokenizerState>>
    return this->func(s);
  }

  /**
   * @brief Binds a transformation function to the current Tokenizer.
   *
   * Allows chaining of Tokenizers by applying a transformation function
   * to the result of the current Tokenizer. The transformation function takes a
   * value of type T and returns a new Tokenizer of type U.
   *
   * @tparam U The type of the value produced by the new Tokenizer.
   * @param f A transformation function that takes a value of type T and returns
   * a Tokenizer of type U.
   * @return A new Tokenizer of type U that applies the transformation function
   * to the result of the current Tokenizer.
   */
  template <typename U>
  Tokenizer<U> bind(std::function<Tokenizer<U>(T)> f) const {
    TokenizerFunc currentFunc = func;
    auto transform = f;
    return Tokenizer<U>([currentFunc = std::move(currentFunc),
                         transform = std::move(transform)](TokenizerState state)
                            -> std::optional<std::pair<U, TokenizerState>> {
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
   * @brief Transforms the result of the current tokenizer.
   *
   * @tparam U The type of the transformed result.
   * @param f A function that takes a value of type T and returns a value of
   * type U.
   * @return Tokenizer<U> A new Tokenizer that applies the transformation
   * function.
   */
  template <typename U> Tokenizer<U> map(std::function<U(T)> f) const {
    return Tokenizer<U>([currentFunc = std::move(func),
                         transform = std::move(f)](TokenizerState state)
                            -> std::optional<std::pair<U, TokenizerState>> {
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
 * @brief Tries the first tokenizer and if it fails, tries the second
 * tokenizer.
 *
 * @tparam T The type of the token produced by the tokenizers (x and y).
 * @param x The first Tokenizer to try.
 * @param y The second Tokenizer to try if the first one fails.
 * @return A new Tokenizer that represents the combination of the two
 * tokenizers.
 *
 * This function attempts to run the first tokenizer with the given state. If
 * the first tokenizer succeeds, it returns the result. If the first tokenizer
 * fails, it attempts to run the second tokenizer. If the second tokenizer
 * succeeds, it returns the result. If both tokenizers fail, it returns nullopt.
 */
template <typename T>
static Tokenizer<T> orElse(Tokenizer<T> x, Tokenizer<T> y) {
  return Tokenizer<T>([x = std::move(x), y = std::move(y)](TokenizerState state)
                          -> std::optional<std::pair<T, TokenizerState>> {
    auto r = x.run(state);
    if (r) {
      return std::make_pair(r->first, r->second);
    }
    r = y.run(state);
    if (r) {
      return std::make_pair(r->first, r->second);
    }
    return std::nullopt;
  });
}

/**
 * @brief Creates a tokenizer that matches zero or more occurrences of the given
 * tokenizer.
 *
 * Returns a tokenizer that repeatedly applies the given tokenizer `x`
 * and collects the results into a vector. The process stops when the tokenizer
 * `x` fails to match or the end of the input string is reached.
 * NEVER returns nullopt. Just consumes if it can.
 *
 * @tparam T The type of tokens produced by the tokenizer `x`.
 * @param x The tokenizer to be applied repeatedly.
 * @return A tokenizer that produces a vector of tokens matched by the given
 * tokenizer `x`.
 */
template <typename T> static Tokenizer<std::vector<T>> manyOf(Tokenizer<T> x) {
  return Tokenizer<std::vector<T>>(
      [x = std::move(x)](TokenizerState state)
          -> std::optional<std::pair<std::vector<T>, TokenizerState>> {
        std::vector<T> gotTokens;
        while (true) {
          if (state.getPosition() >= state.getInputStringSize()) {
            break;
          }
          auto r = x.run(state);
          if (!r) {
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
template <typename T> static Tokenizer<T> pure(T value) {
  return Tokenizer<T>([value = std::move(value)](TokenizerState state)
                          -> std::optional<std::pair<T, TokenizerState>> {
    return std::make_pair(value, state);
  });
}

/** Returns the same state **/
template <typename T>
static Tokenizer<T> match(T value, std::function<bool(T)> matcher) {
  return Tokenizer<T>(
      [matcher = std::move(matcher), value = std::move(value)](
          TokenizerState state) -> std::optional<std::pair<T, TokenizerState>> {
        if (matcher(value)) {
          return pure<T>(value).run(state);
        }
        return std::nullopt;
      });
}

template <typename T> static Tokenizer<T> isEqual(T value, T other) {
  auto equalityChecker = [other = std::move(other)](T val) {
    return val == other;
  };
  return match<T>(value, equalityChecker);
}

template <typename T> static Tokenizer<T> isNotEqual(T value, T other) {
  auto equalityChecker = [other = std::move(other)](T val) {
    return val != other;
  };
  return match<T>(value, equalityChecker);
}

static Tokenizer<char> isDigit(char c) {
  auto digitChecker = [](char c) { return c >= '0' && c <= '9'; };
  return match<char>(c, digitChecker);
}

/** Returns the same state **/
template <typename T> static Tokenizer<T> fail() {
  return Tokenizer<T>(
      [](TokenizerState) -> std::optional<std::pair<T, TokenizerState>> {
        return std::nullopt;
      });
}

static Tokenizer<char> character() {
  return Tokenizer<char>([](TokenizerState state)
                             -> std::optional<std::pair<char, TokenizerState>> {
    if (state.getPosition() >= state.getInputStringSize()) {
      return std::nullopt;
    }
    char val = state.currentCharacter();
    return std::make_pair(val, state.advance());
  });
}

static Tokenizer<char> expectChar(char expected) {
  return Tokenizer<char>([expected = std::move(expected)](TokenizerState state)
                             -> std::optional<std::pair<char, TokenizerState>> {
    if (state.getPosition() >= state.getInputStringSize() ||
        state.currentCharacter() != expected) {
      return std::nullopt;
    }
    return std::make_pair(expected, state.advance());
  });
}

static Tokenizer<std::string> expectString(std::string expected) {
  return Tokenizer<std::string>(
      [expected = std::move(expected)](TokenizerState state)
          -> std::optional<std::pair<std::string, TokenizerState>> {
        for (size_t i = 0; i < expected.size(); i++) {
          auto r = expectChar(expected[i]).run(state);
          if (!r) {
            return std::nullopt;
          }
          state = r->second;
        }
        return std::make_pair(expected, state);
      });
}

[[maybe_unused]]
static Tokenizer<uint> digit() {
  auto asDigit = [](char c) { return static_cast<uint>(c - '0'); };

  return character().bind<uint>(
      [asDigit](char c) { return isDigit(c).map<uint>(asDigit); });
}

static Tokenizer<char> whitespace() { return expectChar(' '); }

[[maybe_unused]]
static Tokenizer<char> braceOpen() {
  return expectChar('{');
}

[[maybe_unused]]
static Tokenizer<char> braceClose() {
  return expectChar('}');
}

[[maybe_unused]]
static Tokenizer<char> bracketOpen() {
  return expectChar('[');
}

[[maybe_unused]]
static Tokenizer<char> bracketClose() {
  return expectChar(']');
}

[[maybe_unused]]
static Tokenizer<char> colon() {
  return expectChar(':');
}

[[maybe_unused]]
static Tokenizer<char> negative() {
  return expectChar('-');
}

[[maybe_unused]]
static Tokenizer<char> doubleQuote() {
  return expectChar('"');
}

[[maybe_unused]]
static Tokenizer<JsonToken> jsonNull() {
  return expectString("null").map<JsonToken>([](std::string) {
    return JsonToken::makeStructural(JsonTokenType::Null);
  });
}

[[maybe_unused]]
static Tokenizer<JsonToken> boolean() {
  return orElse(expectString("true"), expectString("false"))
      .map<JsonToken>([](std::string s) -> JsonToken {
        if (s == "true") {
          return JsonToken::fromBool(true);
        }
        else if (s == "false") {
          return JsonToken::fromBool(false);
        }
        // This should never happen but we need to return something
        return JsonToken::makeNull();
      });
}

static Tokenizer<std::string> word() {
  return manyOf<char>(character().bind<char>([](char c) {
           return (c == ' ' || c == '\t' || c == '\n') ? fail<char>() : pure(c);
         }))
      .bind<std::string>([](std::vector<char> chars) {
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
    return isEqual(c, '{').bind<JsonToken>([](char) {
      return pure(JsonToken::makeStructural(JsonTokenType::ObjectStart));
    });
  });
}

[[maybe_unused]]
static Tokenizer<JsonToken> objectEnd() {
  return character().bind<JsonToken>([](char c) {
    return isEqual(c, '}').bind<JsonToken>([](char) {
      return pure(JsonToken::makeStructural(JsonTokenType::ObjectEnd));
    });
  });
}

[[maybe_unused]]
static Tokenizer<JsonToken> arrayStart() {
  return character().bind<JsonToken>([](char c) {
    return isEqual(c, '[').bind<JsonToken>([](char) {
      return pure(JsonToken::makeStructural(JsonTokenType::ArrayStart));
    });
  });
}

[[maybe_unused]]
static Tokenizer<JsonToken> comma() {
  return character().bind<JsonToken>([](char c) {
    return isEqual(c, ',').bind<JsonToken>([](char) {
      return pure(JsonToken::makeStructural(JsonTokenType::Comma));
    });
  });
}


[[maybe_unused]]
static Tokenizer<JsonToken> arrayEnd() {
  return character().bind<JsonToken>([](char c) {
    return isEqual(c, ']').bind<JsonToken>([](char) {
      return pure(JsonToken::makeStructural(JsonTokenType::ArrayEnd));
    });
  });
}

[[maybe_unused]]
static Tokenizer<JsonToken> jsonString() {
  return doubleQuote()
      .bind<std::string>([](char) {
        return manyOf<char>(character().bind<char>([](char c) {
                 return c == '"' ? fail<char>() : pure(c);
               }))
            .bind<std::string>([](std::vector<char> chars) {
              return doubleQuote().map<std::string>(
                  [chars = std::move(chars)](char) {
                    std::string s(chars.begin(), chars.end());
                    return s;
                  });
            });
      })
      .map<JsonToken>([](std::string s) { return JsonToken::fromString(s); });
}

// TODO implement negatives and exponents
[[maybe_unused]]
static Tokenizer<JsonToken> jsonNumber() {
  return Tokenizer<JsonToken>(
      [](TokenizerState state)
          -> std::optional<std::pair<JsonToken, TokenizerState>> {
        auto negativeResult = negative().run(state);
        bool isNegative = false;
        if (negativeResult) {
          state = negativeResult->second;
          isNegative = true;
        }

        auto r = manyOf<uint>(digit()).run(state);
        if (!r || r->first.size() == 0) {
          return std::nullopt;
        }
        state = r->second;
        auto digits = r->first;
        double number = 0;
        for (int digit : digits) {
          number = number * 10 + digit;
        }
        if (isNegative) {
          number *= -1;
        }

        // Check for decimal point then get rest of number
        r = expectChar('.')
                .bind<std::vector<uint>>(
                    [](char) { return manyOf<uint>(digit()); })
                .run(state);

        if (!r) {
          return std::make_pair(JsonToken::fromNumber(number), state);
        }
        state = r->second;

        digits = r->first;
        double fraction = 0;
        double divisor = 10;
        for (int digit : digits) {
          fraction += digit / divisor;
          divisor *= 10;
        }
        return std::make_pair(JsonToken::fromNumber(number + fraction), state);
      });
}
} // namespace jerry
