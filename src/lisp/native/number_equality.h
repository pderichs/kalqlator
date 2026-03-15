// KalQlator - number_equality.h
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

#include "../factories.h"
#include "../tools.h"

namespace lisp {
const NativeFn FnNumberEquality =
    NativeFn{[](const LispObjectPtr &args, const std::any &) -> LispObjectPtr {
      LispObjectPtr reference;
      LispObjectPtr current = args;

      while (current && !current->is_nil()) {
        if (reference) {
          if (!numeric_equal(reference, current->car())) {
            return make_nil();
          }
        } else {
          reference = current->car();
        }

        current = current->cdr();
      }

      return make_true();
    }};

const NativeFn FnNumbersEqualOrGreater =
    NativeFn{[](const LispObjectPtr &args, const std::any &) -> LispObjectPtr {
      LispObjectPtr previous;
      LispObjectPtr current = args;

      while (current && !current->is_nil()) {
        if (previous) {
          if (previous->as_number() < current->car()->as_number()) {
            return make_nil();
          }
        }

        previous = current->car();
        current = current->cdr();
      }

      return make_true();
    }};

const NativeFn FnNumbersEqualOrSmaller =
    NativeFn{[](const LispObjectPtr &args, const std::any &) -> LispObjectPtr {
      LispObjectPtr previous;
      LispObjectPtr current = args;

      while (current && !current->is_nil()) {
        if (previous) {
          if (previous->as_number() > current->car()->as_number()) {
            return make_nil();
          }
        }

        previous = current->car();
        current = current->cdr();
      }

      return make_true();
    }};
} // namespace lisp
