// KalQlator - calculation.h
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


#include "../object.h"
#include "../NumericOperation.h"

namespace lisp {
    const NativeFn FnAddition = NativeFn{
        [](const LispObjectPtr &args, const std::any&) {
            return numeric_fold(args, std::plus<>{}, 0);
        }
    };

    const NativeFn FnSubtraction = NativeFn{
        [](const LispObjectPtr &args, const std::any&) {
            return numeric_fold(args->cdr(), std::minus<>{}, to_numeric(args->car()));
        }
    };

    const NativeFn FnDivision = NativeFn{
        [](const LispObjectPtr &args, const std::any&) {
            return numeric_fold(args->cdr(), std::divides<>{}, to_numeric(args->car()));
        }
    };

    const NativeFn FnMultiplication = NativeFn{
        [](const LispObjectPtr &args, const std::any&) {
            return numeric_fold(args, std::multiplies<>{}, 1);
        }
    };
}

