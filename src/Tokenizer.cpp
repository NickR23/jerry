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

  template<typename T>
  Tokenizer<T> Tokenizer<T>::init(TokenizerFunc f) {
    Tokenizer<T> toke = Tokenizer();
    toke.func = f;
    return toke;
  }

  template<typename T>
  std::optional<std::pair<T, TokenizerState>> Tokenizer<T>::run(TokenizerState s) {
    // The TokenizerFunc passes it's execution result back up the stack.
    // TokenizerFunc == std::optional<std::pair<T, TokenizerState>>
    return this->func(s);
  }
}
