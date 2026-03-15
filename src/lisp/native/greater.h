// KalQlator - greater.h
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
#include "../object.h"
#include "../tools.h"
#include "../types.h"

namespace lisp {
const NativeFn FnGreater =
    NativeFn{[](const LispObjectPtr &args, const std::any &) -> LispObjectPtr {
      if (countListElements(args) < 1) {
        throw ArgumentError("> requires at least one argument");
      }

      LispObjectPtr first = args->car();
      if (args->cdr()->is_nil()) {
        return make_true();
      }

      LispObjectPtr second = args->cdr()->car();

      if (numeric_greater(first, second)) {
        return make_true();
      }

      return make_nil();
    }};
} // namespace lisp
