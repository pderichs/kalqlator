// KalQlator - test_lisp_if.cpp
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

#include <QtTest/QtTest>
#include <memory>
#include "TestBase.h"
#include "test_tools.h"
#include "../src/lisp/Evaluator.h"
#include "../src/tools/tools.h"
#include "../src/lisp/DefaultEnvironment.h"
using namespace lisp;
class LispIfTests : public TestBase {
    Q_OBJECT
private slots:
    void if1();
    void if2();
    void if3();
    void if_true_literal();
    void if_false_literal();
    void if_nil_is_false();
    void if_zero_is_true();
    void if_empty_string_is_true();
    void if_list_is_true();
    void if_nested();
    void if_evaluates_only_then_branch();
    void if_evaluates_only_else_branch();
    void if_with_expression_condition();
    void if_with_complex_branches();
    void if_returns_expression_result();
};

void LispIfTests::if1() {
    LispObjectPtrVector lisp = parseAllString("(if (eql 2 2) \"Hello\" 6)");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_string());
    QCOMPARE(result->as_string(), "Hello");
}

void LispIfTests::if2() {
    LispObjectPtrVector lisp = parseAllString("(if (eql 3 2) \"Hello\" 6)");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_integer());
    QCOMPARE(result->as_int64(), 6);
}

void LispIfTests::if3() {
    LispObjectPtrVector lisp = parseAllString("(if (eql 3 2) \"Hello\")");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_nil());
}

void LispIfTests::if_true_literal() {
    LispObjectPtrVector lisp = parseAllString("(if t \"yes\" \"no\")");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_string());
    QCOMPARE(result->as_string(), std::string("yes"));
}

void LispIfTests::if_false_literal() {
    // In Common Lisp gibt es kein false-Literal, nur nil
    LispObjectPtrVector lisp = parseAllString("(if nil \"yes\" \"no\")");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_string());
    QCOMPARE(result->as_string(), std::string("no"));
}

void LispIfTests::if_nil_is_false() {
    // nil ist der einzige falsy-Wert in Common Lisp
    LispObjectPtrVector lisp = parseAllString("(if nil 1 2)");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_integer());
    QCOMPARE(result->as_int64(), 2);
}

void LispIfTests::if_zero_is_true() {
    LispObjectPtrVector lisp = parseAllString("(if 0 \"truthy\" \"falsy\")");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_string());
    QCOMPARE(result->as_string(), std::string("truthy"));
}

void LispIfTests::if_empty_string_is_true() {
    // Leerer String ist truthy in Lisp
    LispObjectPtrVector lisp = parseAllString("(if \"\" \"truthy\" \"falsy\")");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_string());
    QCOMPARE(result->as_string(), std::string("truthy"));
}

void LispIfTests::if_list_is_true() {
    // Nicht-leere Liste ist truthy
    LispObjectPtrVector lisp = parseAllString("(if '(1 2 3) \"truthy\" \"falsy\")");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_string());
    QCOMPARE(result->as_string(), std::string("truthy"));
}

void LispIfTests::if_nested() {
    LispObjectPtrVector lisp = parseAllString("(if t (if nil 1 2) 3)");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_integer());
    QCOMPARE(result->as_int64(), 2);
}

void LispIfTests::if_evaluates_only_then_branch() {
    // Else-Branch sollte nicht evaluiert werden, wenn Bedingung true ist
    // Wenn der else-Branch evaluiert würde, würde ein Fehler auftreten (undefined symbol)
    LispObjectPtrVector lisp = parseAllString("(if t 42 undefined-symbol)");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_integer());
    QCOMPARE(result->as_int64(), 42);
}

void LispIfTests::if_evaluates_only_else_branch() {
    // Then-Branch sollte nicht evaluiert werden, wenn Bedingung false ist
    LispObjectPtrVector lisp = parseAllString("(if nil undefined-symbol 42)");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_integer());
    QCOMPARE(result->as_int64(), 42);
}

void LispIfTests::if_with_expression_condition() {
    LispObjectPtrVector lisp = parseAllString("(if (> 5 3) \"greater\" \"smaller\")");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_string());
    QCOMPARE(result->as_string(), std::string("greater"));
}

void LispIfTests::if_with_complex_branches() {
    LispObjectPtrVector lisp = parseAllString("(if t (+ 1 2 3) (* 4 5 6))");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_integer());
    QCOMPARE(result->as_int64(), 6);
}

void LispIfTests::if_returns_expression_result() {
    // if kann in anderen Ausdrücken verwendet werden
    LispObjectPtrVector lisp = parseAllString("(+ 10 (if t 5 0))");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_integer());
    QCOMPARE(result->as_int64(), 15);
}

QTEST_MAIN(LispIfTests)
#include "test_lisp_if.moc"