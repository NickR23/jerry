#include "Tokenizer.h"
#include <cstddef>
#include<iostream>

namespace jerry {
  TokenizerState::TokenizerState(std::string s, size_t pos) : input(s), position(pos){};

  char TokenizerState::currentCharacter() const {
    return input[position];
  }

  //TokenizerState advance() const {}

  Tokenizer::Tokenizer(){};
}
