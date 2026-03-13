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

namespace lisp {
    // Enum for expected types
    enum class ExpectedType {
        t_symbol,
        t_integer,
        t_double,
        t_string,
    };

    // Element is either a type or a value
    using CheckValue = std::variant<ExpectedType, int, std::string, double, Symbol, Cons, Nil, const char*>;

    /**
     * Checks a simple cons for its content. Used within tests.
     *
     * @param obj LispObjectPtr instance to be checked
     * @param expected List of pairs: ExpectedType, value, ExpectedType, ...
     */
    inline void checkCons(const LispObjectPtr& obj, std::initializer_list<CheckValue> expected) {
        QVERIFY(obj->is_cons());

        const auto *it = expected.begin();
        LispObjectPtr current = obj;

        while (it != expected.end() && current->is_cons()) {
            // Type check
            if (!std::holds_alternative<ExpectedType>(*it)) {
                throw std::runtime_error("Expected type specifier");
            }
            ExpectedType type = std::get<ExpectedType>(*it);
            ++it;

            if (it == expected.end()) {
                throw std::runtime_error("Missing value after type");
            }

            LispObjectPtr element = current->car();

            // Check value for type
            switch (type) {
                case ExpectedType::t_symbol:
                    QVERIFY(element->is_symbol());
                    if (const auto* s = std::get_if<std::string>(&*it)) {
                        QCOMPARE(element->as_symbol_name(), *s);
                    } else if (const auto* cs = std::get_if<const char*>(&*it)) {
                        QCOMPARE(element->as_symbol_name(), *cs);
                    }
                    break;

                case ExpectedType::t_string:
                    QVERIFY(element->is_string());
                    if (const auto* s = std::get_if<std::string>(&*it)) {
                        QCOMPARE(element->as_string(), *s);
                    } else if (const auto* cs = std::get_if<const char*>(&*it)) {
                        QCOMPARE(element->as_string(), *cs);
                    }
                    break;

                case ExpectedType::t_integer:
                    QVERIFY(element->is_integer());
                    if (const auto* i = std::get_if<int>(&*it)) {
                        QCOMPARE(element->as_int64(), *i);
                    }
                    break;

                case ExpectedType::t_double:
                    QVERIFY(!element->is_double());
                    if (const auto* i = std::get_if<double>(&*it)) {
                        QCOMPARE(element->as_double(), *i);
                    }
                    break;

                default:
                    QFAIL("Unknown ExpectedType");
                    break;
            }

            ++it;
            current = current->cdr();
        }

        // Everything must be evaluated here
        if (it != expected.end()) {
            QFAIL("More expected elements than in cons");
        }

        if (current->is_cons()) {
            QFAIL("Cons has more elements than expected");
        }
    }
}

