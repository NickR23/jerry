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
    explicit TokenizerState(std::string s, size_t pos);
  public:
    static std::optional<TokenizerState> init(std::string s, size_t pos);

    char currentCharacter() const;
    size_t getPosition() const;
    std::optional<TokenizerState> advance() const;
  };

  template<typename T>
  class Tokenizer {
  private:
    // Used during bind operations. 
    using TokenizerFunc = std::function<std::optional<std::pair<T, TokenizerState>>(TokenizerState)>;
    TokenizerFunc func;
  public:
    explicit Tokenizer(TokenizerFunc f);
  };
}
