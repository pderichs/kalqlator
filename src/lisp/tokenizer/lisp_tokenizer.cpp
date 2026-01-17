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

#include "lisp_tokenizer.h"
#include "lisp_parser_error.h"
#include "lisp_tokens.h"
#include <cctype>
#include <sstream>
#include <string>
#include <utility>
#include "../../tools/tools.h"
using namespace lisp;

Tokenizer::Tokenizer(std::string lisp) : lisp_(std::move(lisp)), pos_(0) {}

LispTokens Tokenizer::scan() {
  LispTokens result;

  pos_ = 0;

  do {
    char c = current_char();

    if (c == '"') {
      result.push_back(read_string());
    } else if (c == '\'') {
      // Quote identifier
      result.push_back(create_identifier_token("'"));
    } else if (c == ';') {
      // Advance to newline
      read_comment();
    } else if (c == '-' || c == '+' || c == '.') {
      // Looking forward to check whether we have
      // a negative/positive number or an identifier here.
      char next = peek(1);
      bool is_number = std::isdigit(next);

      if (is_number) {
        result.push_back(read_number());
      } else {
        result.push_back(read_identifier());
      }
    } else if (std::isdigit(current_char())) {
      // We might have a symbol which just starts with a number.
      // Look ahead and decide:
      bool is_number = true;
      size_t advanced = 0;
      while (walk()) {
        advanced++;

        if (std::isspace(static_cast<unsigned char>(current_char())) || current_char() == ')' || current_char() == '\0') {
          break;
        }

        if (!std::isdigit(current_char()) && current_char() != '.') {
          is_number = false;
          break;
        }
      }

      pos_ -= advanced;

      if (is_number) {
        result.push_back(read_number());
      } else {
        result.push_back(read_identifier());
      }
    } else if (std::isspace(current_char())) {
      result.push_back(create_space_token());
    } else if (c == '(') {
      result.push_back(create_open_bracket_token());
    } else if (c == ')') {
      result.push_back(create_close_bracket_token());
    } else if (std::isprint(c)) {
      result.push_back(read_identifier());
    }
  } while (walk());

  return result;
}

void Tokenizer::read_comment() {
  while (walk()) {
    auto c = current_char();
    if (c == '\n') {
      // New line delimits comment
      break;
    }
  }
}

LispToken Tokenizer::create_double_token(DoubleType number) {
  return LispToken{DOUBLE, number};
}

LispToken Tokenizer::create_integer_token(Int64Type number) {
  return LispToken{INTEGER, number};
}

LispToken Tokenizer::create_string_token(const std::string &s) {
  return LispToken{STRING, s};
}

LispToken Tokenizer::create_identifier_token(const std::string &s) {
  return LispToken{IDENTIFIER, s};
}

LispToken Tokenizer::create_open_bracket_token() {
  return LispToken{OPEN_BRACKET, {}};
}

LispToken Tokenizer::create_close_bracket_token() {
  return LispToken{CLOSE_BRACKET, {}};
}

LispToken Tokenizer::create_space_token() { return LispToken{SPACE, {}}; }

LispToken Tokenizer::read_string() {
  std::string s;
  bool found_end = false;

  while (walk()) {
    char c = current_char();

    if (c == '\\') {
      if (!walk()) {
        throw LispParserError("Syntax error: Unexpected end after escape", s);
      }
      char escaped = current_char();
      switch (escaped) {
        case 'n':  s += '\n'; break;
        case 't':  s += '\t'; break;
        case 'r':  s += '\r'; break;
        case '\\': s += '\\'; break;
        case '"':  s += '"';  break;
        default:
          // Unknown escape sequence - add it nevertheless(?)
          s += escaped;
          break;
      }
    } else if (c == '"') {
      found_end = true;
      break;
    } else {
      s += c;
    }
  }

  if (!found_end) {
    throw LispParserError("Syntax error: String is not terminated", s);
  }

  return create_string_token(s);
}

LispToken Tokenizer::read_number() {
  std::string s;
  bool dot = false;

  do {
    char c = current_char();

    if (std::isdigit(c)) {
      s += c;
    } else if (c == '-') {
      if (!s.empty()) {
        throw LispParserError("Unexpected dash while parsing number", s);
      }

      s += c;
    } else if (c == '.') {
      if (dot) {
        throw LispParserError("Second dot while parsing number", s);
      }

      s += c;
      dot = true;
    } else if (std::isspace(c) || c == ')') {
      pos_--;
      break;
    }
  } while (walk());

  auto opt_int = pdtools::convert_string_to_number<Int64Type>(s);
  if (opt_int) {
    return create_integer_token(*opt_int);
  }

  auto opt_double = pdtools::convert_string_to_number<DoubleType>(s);
  if (opt_double) {
    return create_double_token(*opt_double);
  }

  std::stringstream ss;
  ss << "Unable to convert string to number: \"";
  ss << s << "\"";
  throw LispParserError(ss.str());
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
  std::string s;

  do {
    char c = current_char();

    if (std::isspace(c) || c == ')') {
      pos_--;
      break;
    }

    if (std::isprint(c)) {
      s += c;
    } else if (c == 0) {
      break;
    } else {
      throw LispParserError("Not supported: Unprintable char detected.");
    }
  } while (walk());

  return create_identifier_token(s);
}

char Tokenizer::peek(int offset) const {
  if (pos_ + offset >= lisp_.size()) {
    return '\0';
  }

  return lisp_[pos_ + offset];
}
