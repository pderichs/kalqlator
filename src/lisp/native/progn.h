// KalQlator - progn.h
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

namespace lisp {
// Evaluator is already evaluating list parts. So our
// progn can be really simple.
const NativeFn FnProgn =
    NativeFn{[](const LispObjectPtr &args, const std::any &) {
      LispObjectPtr result = make_nil();
      LispObjectPtr current = args;
      while (current && !current->is_nil()) {
        result = current->car();
        current = current->cdr();
      }
      return result;
    }};
} // namespace lisp
