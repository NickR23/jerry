#pragma once
#include <cstddef>
#include <optional>
#include <string>

namespace jerry {
  class TokenizerState {
    public:
    static std::optional<TokenizerState> init(std::string s, size_t pos);

    char currentCharacter() const;
    size_t getPosition() const;
    std::optional<TokenizerState> advance() const;

    private:
    std::string input;
    size_t position;
    explicit TokenizerState(std::string s, size_t pos);
  };

  class Tokenizer {
    public:
    Tokenizer();
  };

}
