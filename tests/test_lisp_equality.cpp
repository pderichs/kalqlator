// KalQlator - test_lisp_equality.cpp
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

#include <memory>
#include <QtTest/QtTest>
#include "TestBase.h"
#include "../src/lisp/DefaultEnvironment.h"
#include "../src/lisp/Evaluator.h"
#include "../src/lisp/tools.h"
#include "../src/tools/tools.h"

using namespace lisp;
class LispEqualityTests : public TestBase {
    Q_OBJECT
private slots:
    // eql tests
    static void eql_ints_equal();

    static void eql_ints_not_equal();

    static void eql_lists_equal();

    static void eql_lists_different_value();

    static void eql_lists_different_length();

    // eq tests
    static void eq_symbols_same();

    static void eq_symbols_different();

    static void eq_same_variable();

    static void eq_different_lists_same_content();

    // = (numeric equality) tests
    static void numeric_eq_two_equal();

    static void numeric_eq_two_not_equal();

    static void numeric_eq_multiple_all_equal();

    static void numeric_eq_multiple_one_different();

    static void numeric_eq_single_value();

    static void numeric_eq_floats();

    static void numeric_eq_mixed_int_float();
};

// === eql tests ===

void LispEqualityTests::eql_ints_equal() {
    LispObjectPtrVector lisp = parse_all_string("(eql 45 45)");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_true());
}

void LispEqualityTests::eql_ints_not_equal() {
    LispObjectPtrVector lisp = parse_all_string("(eql 45 46)");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_false());
}

void LispEqualityTests::eql_lists_equal() {
    LispObjectPtrVector lisp = parse_all_string("(eql '(10 22 53) '(10 22 53))");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_true());
}

void LispEqualityTests::eql_lists_different_value() {
    LispObjectPtrVector lisp = parse_all_string("(eql '(10 22 53) '(10 22 52))");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_false());
}

void LispEqualityTests::eql_lists_different_length() {
    LispObjectPtrVector lisp = parse_all_string("(eql '(10 22) '(10 22 52))");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_false());
}

// === eq tests ===

void LispEqualityTests::eq_symbols_same() {
    LispObjectPtrVector lisp = parse_all_string("(eq 'foo 'foo)");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_true());
}

void LispEqualityTests::eq_symbols_different() {
    LispObjectPtrVector lisp = parse_all_string("(eq 'foo 'bar)");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_false());
}

void LispEqualityTests::eq_same_variable() {
    // Eine Liste, zwei Referenzen -> eq sollte T sein
    LispObjectPtrVector lisp = parse_all_string("((lambda (x) (eq x x)) '(1 2 3))");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_true());
}

void LispEqualityTests::eq_different_lists_same_content() {
    // Zwei verschiedene Listen mit gleichem Inhalt -> eq sollte NIL sein
    LispObjectPtrVector lisp = parse_all_string("(eq '(1 2 3) '(1 2 3))");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_false());
}

// === = (numeric equality) tests ===

void LispEqualityTests::numeric_eq_two_equal() {
    LispObjectPtrVector lisp = parse_all_string("(= 42 42)");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_true());
}

void LispEqualityTests::numeric_eq_two_not_equal() {
    LispObjectPtrVector lisp = parse_all_string("(= 42 43)");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_false());
}

void LispEqualityTests::numeric_eq_multiple_all_equal() {
    LispObjectPtrVector lisp = parse_all_string("(= 7 7 7 7 7)");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_true());
}

void LispEqualityTests::numeric_eq_multiple_one_different() {
    LispObjectPtrVector lisp = parse_all_string("(= 1 1 1 1 4)");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_false());
}

void LispEqualityTests::numeric_eq_single_value() {
    // Ein Argument -> immer T
    LispObjectPtrVector lisp = parse_all_string("(= 99)");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_true());
}

void LispEqualityTests::numeric_eq_floats() {
    LispObjectPtrVector lisp = parse_all_string("(= 3.14 3.14)");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_true());
}

void LispEqualityTests::numeric_eq_mixed_int_float() {
    // 5 und 5.0 sollten numerisch gleich sein
    LispObjectPtrVector lisp = parse_all_string("(= 5 5.0)");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_true());
}

QTEST_MAIN(LispEqualityTests)
#include "test_lisp_equality.moc"