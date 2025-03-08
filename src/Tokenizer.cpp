#include "Tokenizer.h"
#include <cstddef>
#include<iostream>

namespace jerry {
  TokenizerState::TokenizerState(std::string s, size_t pos) : input(s), position(pos){};

  TokenizerState TokenizerState::init(std::string s, size_t pos) {
    return TokenizerState(s, pos);
  }

  size_t TokenizerState::getPosition() const {
    return position;
  }

  char TokenizerState::currentCharacter() const {
    return input[position];
  }

  TokenizerState TokenizerState::advance() const {
    return init(input, position + 1);
  }
}
