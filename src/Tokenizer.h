#pragma once
#include <cstddef>
#include <string>

namespace jerry {
  class TokenizerState {
    public:
    TokenizerState() = default;
    explicit TokenizerState(std::string s, size_t pos);
    char currentCharacter() const;
    TokenizerState advance() const;

    private:
    std::string input;
    size_t position;
  };

  class Tokenizer {
    public:
    Tokenizer();
  };

}
