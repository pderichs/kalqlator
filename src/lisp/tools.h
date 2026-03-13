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

#include "types.h"
#include "object.h"
#include "TypeError.h"

#include "tokenizer/lisp_tokenizer.h"
#include "parser/parser.h"

namespace lisp {
    using NumericValue = std::variant<Int64Type, DoubleType>;

    inline NumericValue to_numeric(const LispObjectPtr &value) {
        if (value->is_integer()) { return value->as_int64(); }
        if (value->is_double()) { return value->as_double(); }
        if (value->is_nil()) { return 0; }
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

    template<typename Op = std::equal_to<>>
    inline bool numeric_compare(const LispObjectPtr &first, const LispObjectPtr &second, Op operation = {}) {
        return std::visit([&operation](auto &&lhs, auto &&rhs) -> bool {
            if constexpr (std::is_arithmetic_v<std::decay_t<decltype(lhs)>> &&
                          std::is_arithmetic_v<std::decay_t<decltype(rhs)>>) {
                return operation(lhs, rhs);
            }
            return false;
        }, first->data, second->data);
    }

    inline bool numeric_equal(const LispObjectPtr &first, const LispObjectPtr &second) {
        return numeric_compare(first, second, std::equal_to<>{});
    }

    inline bool numeric_greater(const LispObjectPtr &first, const LispObjectPtr &second) {
        return std::visit([](auto &&lhs, auto &&rhs) -> bool {
            if constexpr (std::is_arithmetic_v<std::decay_t<decltype(lhs)> > &&
                          std::is_arithmetic_v<std::decay_t<decltype(rhs)> >) {
                // Using promotion
                return lhs > rhs;
            }
            return false;
        }, first->data, second->data);
    }

    inline bool numeric_smaller(const LispObjectPtr &first, const LispObjectPtr &second) {
        return std::visit([](auto &&lhs, auto &&rhs) -> bool {
            if constexpr (std::is_arithmetic_v<std::decay_t<decltype(lhs)> > &&
                          std::is_arithmetic_v<std::decay_t<decltype(rhs)> >) {
                // Using promotion
                return lhs < rhs;
            }
            return false;
        }, first->data, second->data);
    }

    inline LispObjectPtrVector parse_all_string(const std::string &input) {
        Tokenizer tokenizer(input);
        LispTokens tokens = tokenizer.scan();
        Parser parser(tokens);
        return parser.parse_all();
    }

}
