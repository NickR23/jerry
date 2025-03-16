#include <cstddef>
#include <iostream>

#include "Tokenizer.h"

namespace jerry {
TokenizerState::TokenizerState(std::string s, size_t pos)
    : input(s), position(pos) {};

TokenizerState TokenizerState::init(std::string s, size_t pos) {
  return TokenizerState(s, pos);
}

size_t TokenizerState::getPosition() const noexcept { return position; }

char TokenizerState::currentCharacter() const noexcept {
  return input[position];
}

TokenizerState TokenizerState::advance() const {
  return init(input, position + 1);
}
}  // namespace jerry
