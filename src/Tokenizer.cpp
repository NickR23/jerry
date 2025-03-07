#include "Tokenizer.h"
#include <cstddef>
#include<iostream>

namespace jerry {
  TokenizerState::TokenizerState(std::string s, size_t pos) : input(s), position(pos){};

  std::optional<TokenizerState> TokenizerState::init(std::string s, size_t pos) {
    if (pos >= s.size()) {
      return std::nullopt;
    }
    return TokenizerState(s,pos);
  }

  char TokenizerState::currentCharacter() const {
    return input[position];
  }

  //TokenizerState advance() const {}

  Tokenizer::Tokenizer(){};
}
