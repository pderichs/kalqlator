// KalQlator - lisp_tokenizer.cpp
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

#include "tokenizer.h"
#include "../parser/parser_error.h"
#include "tokens.h"
#include <cctype>
#include <gmpxx.h>
#include <sstream>
#include <string>
#include <utility>
#include "../../tools/tools.h"

using namespace lisp;

Tokenizer::Tokenizer(std::string lisp) : lisp_(std::move(lisp)) {
}

bool Tokenizer::digit_sequence_is_number() {
  size_t advanced = 0;
  bool result = true;

  while (walk()) {
    advanced++;
    const char current = current_char();

    if (std::isspace(static_cast<unsigned char>(current)) != 0 || current == ')' || current == '\0') {
      break;
    }
    if (std::isdigit(current) == 0 && current != '.') {
      result = false;
      break;
    }
  }

  pos_ -= advanced;
  return result;
}

LispTokens Tokenizer::scan() {
  LispTokens result;
  pos_ = 0;

  do {
    char current = current_char();

    switch (current) {
      case '"':  result.push_back(read_string());                       break;
      case '\'': result.push_back(create_identifier_token("'"));    break;
      case ';':  read_comment();                                          break;
      case '(':  result.push_back(create_open_bracket_token());         break;
      case ')':  result.push_back(create_close_bracket_token());        break;

      case '-': case '+': case '.':
        result.push_back(std::isdigit(peek(1)) != 0 ? read_number() : read_identifier());
        break;

      default:
        if (std::isdigit(current) != 0) {
          result.push_back(digit_sequence_is_number() ? read_number() : read_identifier());
        } else if (std::isspace(current) != 0) {
          result.push_back(create_space_token());
        } else if (std::isprint(current) != 0) {
          result.push_back(read_identifier());
        }

        break;
    }
  } while (walk());

  return result;
}

void Tokenizer::read_comment() {
  while (walk()) {
    auto current = current_char();
    if (current == '\n') {
      // New line delimits comment
      break;
    }
  }
}

LispToken Tokenizer::create_number_token(const std::string& number) {
  return LispToken{.id=NUMBER, .content=number};
}

LispToken Tokenizer::create_string_token(const std::string &value) {
  return LispToken{.id=STRING, .content=value};
}

LispToken Tokenizer::create_identifier_token(const std::string &value) {
  return LispToken{.id=IDENTIFIER, .content = value};
}

LispToken Tokenizer::create_open_bracket_token() {
  return LispToken{.id=OPEN_BRACKET, .content={}};
}

LispToken Tokenizer::create_close_bracket_token() {
  return LispToken{.id=CLOSE_BRACKET, .content={}};
}

LispToken Tokenizer::create_space_token() { return LispToken{.id=SPACE, .content={}}; }

LispToken Tokenizer::read_string() {
  std::string string;
  bool found_end = false;

  while (walk()) {
    const char current = current_char();

    if (current == '\\') {
      if (!walk()) {
        throw LispParserError("Syntax error: Unexpected end after escape", string);
      }
      char escaped = current_char();
      switch (escaped) {
        case 'n':  string += '\n'; break;
        case 't':  string += '\t'; break;
        case 'r':  string += '\r'; break;
        case '\\': string += '\\'; break;
        case '"':  string += '"';  break;
        default:
          // Unknown escape sequence - add it nevertheless(?)
          string += escaped;
          break;
      }
    } else if (current == '"') {
      found_end = true;
      break;
    } else {
      string += current;
    }
  }

  if (!found_end) {
    throw LispParserError("Syntax error: String is not terminated", string);
  }

  return create_string_token(string);
}

LispToken Tokenizer::read_number() {
  std::string number_string;
  bool dot = false;

  do {
    const char current = current_char();

    if (std::isdigit(current) != 0) {
      number_string += current;
    } else if (current == '-') {
      if (!number_string.empty()) {
        throw LispParserError("Unexpected dash while parsing number", number_string);
      }

      number_string += current;
    } else if (current == '.') {
      if (dot) {
        throw LispParserError("Second dot while parsing number", number_string);
      }

      number_string += current;
      dot = true;
    } else if (std::isspace(current) != 0 || current == ')') {
      pos_--;
      break;
    }
  } while (walk());

  bool number_ok = false;
  auto result = pdtools::convert_string_to_number<int64_t>(number_string);
  if (result) {
    number_ok = true;
  }

  if (!number_ok) {
    result = pdtools::convert_string_to_number<double>(number_string);
    if (result) {
      number_ok = true;
    }
  }

  if (!number_ok) {
    std::stringstream error_message;
    error_message << "Unable to convert string to number: \"";
    error_message << number_string << "\"";
    throw LispParserError(error_message.str());
  }

  return create_number_token(number_string);
}

bool Tokenizer::walk() {
  if (pos_ >= lisp_.size()) {
    return false;
  }

  pos_++;

  return true;
}

char Tokenizer::current_char() const { return lisp_[pos_]; }

LispToken Tokenizer::read_identifier() {
  std::string identifier;

  do {
    const char current = current_char();

    if (std::isspace(current) != 0 || current == ')') {
      pos_--;
      break;
    }

    if (std::isprint(current) != 0) {
      identifier += current;
    } else if (current == 0) {
      break;
    } else {
      throw LispParserError("Not supported: Unprintable char detected.");
    }
  } while (walk());

  return create_identifier_token(identifier);
}

char Tokenizer::peek(const int offset) const {
  if (pos_ + offset >= lisp_.size()) {
    return '\0';
  }

  return lisp_[pos_ + offset];
}
