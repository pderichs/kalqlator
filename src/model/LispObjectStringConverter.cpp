// KalQlator - LispObjectStringConverter.cpp
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

#include "LispObjectStringConverter.h"

#include "../lisp/object.h"


LispObjectStringConverter::LispObjectStringConverter(lisp::LispObjectPtr object) : object_(std::move(object)) {
}

std::string LispObjectStringConverter::to_str() const {
    std::string result;

    if (object_->is_double()) {
        result = std::to_string(object_->as_double());
    } else if (object_->is_integer()) {
        result = std::to_string(object_->as_int64());
    } else if (object_->is_string()) {
        result = object_->as_string();
    } else if (object_->is_symbol()) {
        result = object_->as_symbol_name();
    } else {
        result = "<value>";
    }

    return result;
}
