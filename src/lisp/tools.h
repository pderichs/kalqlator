// KalQlator - tools.h
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

#include "TypeError.h"
#include "object.h"
#include "types.h"

#include "parser/parser.h"
#include "tokenizer/tokenizer.h"

namespace lisp {
// TODO: Eval
inline mpq_class to_numeric(const LispObjectPtr &value,
                            mpq_class default_value = {0}) {
  if (value->is_number()) {
    return mpq_class(value->as_number());
  }
  if (value->is_nil()) {
    return default_value;
  }
  throw TypeError("Only numbers are allowed.");
}

inline size_t countListElements(const LispObjectPtr &list) {
  size_t count = 0;
  LispObjectPtr current = list;
  while (current && current->is_cons()) {
    count++;
    current = current->cdr();
  }
  return count;
}

inline bool numeric_equal(const LispObjectPtr &first,
                          const LispObjectPtr &second) {
  return first->as_number() == second->as_number();
}

inline bool numeric_greater(const LispObjectPtr &first,
                            const LispObjectPtr &second) {
  return first->as_number() > second->as_number();
}

inline bool numeric_smaller(const LispObjectPtr &first,
                            const LispObjectPtr &second) {
  return first->as_number() < second->as_number();
}

inline LispObjectPtrVector parse_all_string(const std::string &input) {
  Tokenizer tokenizer(input);
  LispTokens tokens = tokenizer.scan();
  Parser parser(tokens);
  return parser.parse_all();
}

} // namespace lisp
