// KalQlator - lisp_tokenizer.h
// Copyright (C) 2026  pderichs
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "tokens.h"
#include <cstddef>
#include <string>

#include "../types.h"

namespace lisp {
  /**
   * Handles all the logic to convert a string to a list of tokens (Tokenizer).
   */
  class Tokenizer {
  public:
    explicit Tokenizer(std::string lisp);

    LispTokens scan();

  private:
    bool digit_sequence_is_number();

    void read_comment();

    static LispToken create_number_token(const std::string& number);

    static LispToken create_string_token(const std::string &value);

    static LispToken create_identifier_token(const std::string &value);

    static LispToken create_open_bracket_token();

    static LispToken create_close_bracket_token();

    static LispToken create_space_token();

    LispToken read_string();
    LispToken read_number();
    LispToken read_identifier();
    bool walk();
    [[nodiscard]] char current_char() const;

    [[nodiscard]] char peek(int offset) const;

  private:
    std::string lisp_;
    size_t pos_{0};
  };
}

