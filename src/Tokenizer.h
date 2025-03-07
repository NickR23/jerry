#pragma once
#include <cstddef>
#include <optional>
#include <string>
#include <functional>

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
    static Tokenizer<T> init(TokenizerFunc f);

    std::optional<std::pair<T, TokenizerState>> run(TokenizerState s) {
      // The TokenizerFunc passes it's execution result back up the stack.
      // TokenizerFunc == std::optional<std::pair<T, TokenizerState>>
      return this->func(s);
    }
  };
}
