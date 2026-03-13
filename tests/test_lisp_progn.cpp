// KalQlator - test_lisp_progn.cpp
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
#include "../src/lisp/Evaluator.h"
#include "../src/lisp/tools.h"
#include "../src/tools/tools.h"
#include "../src/lisp/DefaultEnvironment.h"

using namespace lisp;
class LispPrognTests : public TestBase {
    Q_OBJECT
private slots:
    static void progn_empty();

    static void progn_single_expression();

    static void progn_multiple_expressions();

    static void progn_returns_last();

    static void progn_nested();

    static void progn_with_arithmetic();
};

void LispPrognTests::progn_empty() {
    LispObjectPtrVector lisp = parse_all_string("(progn)");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_nil());
}

void LispPrognTests::progn_single_expression() {
    LispObjectPtrVector lisp = parse_all_string("(progn 42)");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QCOMPARE(result->as_int64(), 42);
}

void LispPrognTests::progn_multiple_expressions() {
    LispObjectPtrVector lisp = parse_all_string("(progn 1 2 3)");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QCOMPARE(result->as_int64(), 3);
}

void LispPrognTests::progn_returns_last() {
    LispObjectPtrVector lisp = parse_all_string("(progn (+ 1 1) (+ 2 2) (+ 3 3))");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QCOMPARE(result->as_int64(), 6);
}

void LispPrognTests::progn_nested() {
    LispObjectPtrVector lisp = parse_all_string("(progn (progn 1 2) (progn 3 4))");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QCOMPARE(result->as_int64(), 4);
}

void LispPrognTests::progn_with_arithmetic() {
    LispObjectPtrVector lisp = parse_all_string("(progn (* 2 3) (- 10 5) (+ 100 23))");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QCOMPARE(result->as_int64(), 123);
}

QTEST_MAIN(LispPrognTests)
#include "test_lisp_progn.moc"