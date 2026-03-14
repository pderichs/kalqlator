// KalQlator - lisp_syntax_checker.cpp
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

#include <sstream>

#include "syntax_checker.h"
#include "../parser/parser_error.h"

using lisp::LispSyntaxChecker;

LispSyntaxChecker::LispSyntaxChecker(LispTokens tokens)
    : tokens_(std::move(tokens)) {}

void LispSyntaxChecker::check() const {
  check_matching_brackets();
}

void LispSyntaxChecker::check_matching_brackets() const {
  int open_brackets = 0;
  int closed_brackets = 0;

  bool expecting_open_bracket = true;

  for (const auto &token : tokens_) {
    if (token.is_open_bracket()) {
      expecting_open_bracket = false;
      open_brackets++;
    } else if (token.is_closed_bracket()) {
      if (expecting_open_bracket) {
        throw LispParserError("Unexpected closing bracket");
      }

      closed_brackets++;

      if (closed_brackets > open_brackets) {
        throw LispParserError("Unexpected closing bracket (2)");
      }
    }
  }

  if (open_brackets != closed_brackets) {
    std::stringstream error_message;
    error_message << "Brackets count mismatch: " << open_brackets << "/"
       << closed_brackets;
    throw LispParserError(error_message.str());
  }
}

