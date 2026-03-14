// KalQlator - NumericOperation.h
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


#include <variant>

#include "factories.h"
#include "object.h"
#include "tools.h"
#include "TypeError.h"
#include "types.h"

namespace lisp {
    template<typename Op>
    LispObjectPtr numeric_fold(const LispObjectPtr &args, Op operation, mpq_class start_value) {
        mpq_class acc = start_value;
        LispObjectPtr current = args;

        while (current && !current->is_nil()) {
            acc = operation(acc, to_numeric(current->car()));
            current = current->cdr();
        }

        return make_number(NumberRepresentation{ acc.get_str() });
    }
} // lisp
