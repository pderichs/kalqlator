// KalQlator - test_tools.h
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


#include <QtTest/QtTest>
#include <stdexcept>
#include <variant>

#include "../src/lisp/types.h"
#include "../src/lisp/object.h"
#include "../src/lisp/tools.h"

namespace lisp {
    // Enum for expected types
    enum class ExpectedType {
        t_symbol,
        t_number,
        t_string,
    };

    // Element is either a type or a value
    using CheckValue = std::variant<ExpectedType, std::string, Symbol, Cons, Nil, const char*, mpq_class>;

    /**
     * Checks a simple cons for its content. Used within tests.
     *
     * @param obj LispObjectPtr instance to be checked
     * @param expected List of pairs: ExpectedType, value, ExpectedType, ...
     */
    inline void checkCons(const LispObjectPtr& obj, std::initializer_list<CheckValue> expected) {
        QVERIFY(obj->is_cons());

        const auto *iterator = expected.begin();
        LispObjectPtr current = obj;

        while (iterator != expected.end() && current->is_cons()) {
            // Type check
            if (!std::holds_alternative<ExpectedType>(*iterator)) {
                throw std::runtime_error("Expected type specifier");
            }
            ExpectedType type = std::get<ExpectedType>(*iterator);
            ++iterator;

            if (iterator == expected.end()) {
                throw std::runtime_error("Missing value after type");
            }

            LispObjectPtr element = current->car();

            // Check value for type
            switch (type) {
                case ExpectedType::t_symbol:
                    QVERIFY(element->is_symbol());
                    if (const auto* string = std::get_if<std::string>(iterator)) {
                        QCOMPARE(element->as_symbol_name(), *string);
                    } else if (const auto* c_str = std::get_if<const char*>(iterator)) {
                        QCOMPARE(element->as_symbol_name(), *c_str);
                    } else {
                        QFAIL("Unknown type (t_symbol)");
                    }
                    break;

                case ExpectedType::t_string:
                    QVERIFY(element->is_string());
                    if (const auto* string = std::get_if<std::string>(iterator)) {
                        QCOMPARE(element->as_string(), *string);
                    } else if (const auto* c_str = std::get_if<const char*>(iterator)) {
                        QCOMPARE(element->as_string(), *c_str);
                    } else {
                        QFAIL("Unknown type (t_string)");
                    }

                    break;

                case ExpectedType::t_number:
                    QVERIFY(element->is_number());
                    if (const auto *const value = std::get_if<mpq_class>(iterator)) {
                        QCOMPARE(to_numeric(element), *value);
                    } else {
                        QFAIL("Unknown type (t_number)");
                    }

                    break;

                default:
                    QFAIL("Unknown ExpectedType");
                    break;
            }

            ++iterator;
            current = current->cdr();
        }

        // Everything must be evaluated here
        if (iterator != expected.end()) {
            QFAIL("More expected elements than in cons");
        }

        if (current->is_cons()) {
            QFAIL("Cons has more elements than expected");
        }
    }
}

