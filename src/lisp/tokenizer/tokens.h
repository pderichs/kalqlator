// KalQlator - lisp_tokens.h
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

#include <any>
#include <vector>

namespace lisp {
  /**
   * Each token has a specific id. The tokenizer implements the logic to
   * identify these.
   */
  enum TokenID {
    OPEN_BRACKET,
    CLOSE_BRACKET,
    IDENTIFIER,
    STRING,
    NUMBER,
    SPACE
  };

  /**
   * Combines a token id with the actual content from the string.
   */
  struct LispToken {
    TokenID id;
    std::any content;

    [[nodiscard]] bool is_space() const { return id == SPACE; }
    [[nodiscard]] bool is_identifier() const { return id == IDENTIFIER; }
    [[nodiscard]] bool is_string() const { return id == STRING; }
    [[nodiscard]] bool is_number() const { return id == NUMBER; }
    [[nodiscard]] bool is_open_bracket() const { return id == OPEN_BRACKET; }
    [[nodiscard]] bool is_closed_bracket() const { return id == CLOSE_BRACKET; }
  };

  using LispTokens = std::vector<LispToken>;
}

