// KalQlator - eq.h
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

#include "../ArgumentError.h"
#include "../factories.h"
#include "../tools.h"

namespace lisp {
bool is_equal(const LispObjectPtr &first, const LispObjectPtr &second);

inline bool list_equal(const LispObjectPtr &first,
                       const LispObjectPtr &second) {
  if (countListElements(first) != countListElements(second)) {
    return false;
  }

  LispObjectPtr it_a = first;
  LispObjectPtr it_b = second;
  while (it_a && !it_a->is_nil() && it_b && !it_b->is_nil()) {
    if (!is_equal(it_a->car(), it_b->car())) {
      return false;
    }

    it_a = it_a->cdr();
    it_b = it_b->cdr();
  }

  return true;
}

inline bool is_equal(const LispObjectPtr &first, const LispObjectPtr &second) {
  // Check addresses
  if (first == second) {
    return true;
  }

  if (first->is_nil() && second->is_nil()) {
    return true;
  }

  if (first->is_true() && second->is_true()) {
    return true;
  }

  if (first->is_false() && second->is_false()) {
    return true;
  }

  if (first->is_symbol() && second->is_symbol()) {
    return first->as_symbol_name() == second->as_symbol_name();
  }

  if (first->is_string() && second->is_string()) {
    return first->as_string() == second->as_string();
  }

  if (first->is_number() && second->is_number()) {
    return numeric_equal(first, second);
  }

  if (first->is_cons() && second->is_cons()) {
    return list_equal(first, second);
  }

  return false;
}

const NativeFn FnEq =
    NativeFn{[](const LispObjectPtr &args, const std::any &) -> LispObjectPtr {
      if (countListElements(args) != 2) {
        throw ArgumentError("eq requires only 2 args");
      }

      const LispObjectPtr first = args->car();
      const LispObjectPtr second = args->cdr()->car();

      if (first->is_symbol() && second->is_symbol()) {
        return make_bool(is_equal(first, second));
      }

      // TODO Check if this is okay
      return make_bool(first == second);
    }};

/**
 * This defines the function "eql" in Lisp which means comparison for
 * same address or same values.
 */
const NativeFn FnEql =
    NativeFn{[](const LispObjectPtr &args, const std::any &) -> LispObjectPtr {
      if (countListElements(args) != 2) {
        throw ArgumentError("eql just accepts two args");
      }

      LispObjectPtr first;
      LispObjectPtr second;

      LispObjectPtr current = args;
      while (current && !current->is_nil()) {
        if (!first) {
          first = current->car();
        } else if (!second) {
          second = current->car();
        } else {
          // Check types and equality
          bool equal = is_equal(first, second);
          if (!equal) {
            return make_nil(); // false
          }

          // Move to next pair
          first = second;
          second = nullptr;
        }

        current = current->cdr();
      }

      if (first && second) {
        if (!is_equal(first, second)) {
          return make_nil();
        }
      }

      return make_true();
    }};
} // namespace lisp
