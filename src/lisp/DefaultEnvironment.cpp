// KalQlator - DefaultEnvironment.cpp
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


#include "DefaultEnvironment.h"
#include "native/calculation.h"
#include "native/car.h"
#include "native/cdr.h"
#include "native/eq.h"
#include "native/greater.h"
#include "native/list.h"
#include "native/progn.h"
#include "native/number_equality.h"
#include "native/smaller.h"

namespace lisp {
    DefaultEnvironment::DefaultEnvironment() : Environment(nullptr) {
        // Initialize all default functions
        set("+", make_native_fn(FnAddition));
        set("-", make_native_fn(FnSubtraction));
        set("*", make_native_fn(FnMultiplication));
        set("/", make_native_fn(FnDivision));

        set("=", make_native_fn(FnNumberEquality));

        set("eq", make_native_fn(FnEq));

        set("eql", make_native_fn(FnEql));
        // TODO: "=" function

        set("progn", make_native_fn(FnProgn));

        set("car", make_native_fn(FnCar));
        set("cdr", make_native_fn(FnCdr));

        set(">", make_native_fn(FnGreater));
        set("<", make_native_fn(FnSmaller));

        set(">=", make_native_fn(FnNumbersEqualOrGreater));
        set("<=", make_native_fn(FnNumbersEqualOrSmaller));

        set("list", make_native_fn(FnList));
    }
}
