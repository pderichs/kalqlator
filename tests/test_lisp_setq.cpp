// KalQlator - test_lisp_setq.cpp
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
class LispQuoteTests : public TestBase {
    Q_OBJECT
private slots:
    void setq_lambda();
    void setq_int();
    void setq_string();
    void setq_list();
    void setq_nil();
    void setq_overwrite();
    void setq_expression_result();
    void setq_nested_reference();
    void setq_returns_value();
    void setq_symbol_reference();
};

void LispQuoteTests::setq_lambda() {
    LispObjectPtrVector lisp = parseAllString("(setq my-func (lambda (x y) (eql x y))) (my-func \"a\" \"a\")");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_true());
}

void LispQuoteTests::setq_int() {
    LispObjectPtrVector lisp = parseAllString("(setq answer-to-everything 42) (+ answer-to-everything answer-to-everything answer-to-everything)");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_integer());
    QCOMPARE(result->as_int64(), 126);
}

void LispQuoteTests::setq_string() {
    LispObjectPtrVector lisp = parseAllString("(setq greeting \"Hello, World!\") greeting");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_string());
    QCOMPARE(result->as_string(), std::string("Hello, World!"));
}

void LispQuoteTests::setq_list() {
    LispObjectPtrVector lisp = parseAllString("(setq my-list '(1 2 3)) (car my-list)");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_integer());
    QCOMPARE(result->as_int64(), 1);
}

void LispQuoteTests::setq_nil() {
    LispObjectPtrVector lisp = parseAllString("(setq empty nil) empty");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_nil());
}

void LispQuoteTests::setq_overwrite() {
    LispObjectPtrVector lisp = parseAllString("(setq x 10) (setq x 20) x");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_integer());
    QCOMPARE(result->as_int64(), 20);
}

void LispQuoteTests::setq_expression_result() {
    LispObjectPtrVector lisp = parseAllString("(setq sum (+ 1 2 3 4 5)) sum");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_integer());
    QCOMPARE(result->as_int64(), 15);
}

void LispQuoteTests::setq_nested_reference() {
    LispObjectPtrVector lisp = parseAllString("(setq a 5) (setq b (* a a)) (setq c (+ a b)) c");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_integer());
    QCOMPARE(result->as_int64(), 30);  // a=5, b=25, c=30
}

void LispQuoteTests::setq_returns_value() {
    // setq sollte den zugewiesenen Wert zurückgeben
    LispObjectPtrVector lisp = parseAllString("(+ 1 (setq x 10))");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_integer());
    QCOMPARE(result->as_int64(), 11);
}

void LispQuoteTests::setq_symbol_reference() {
    // Variable referenziert andere Variable
    LispObjectPtrVector lisp = parseAllString("(setq original 42) (setq copy original) (setq original 100) copy");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_integer());
    QCOMPARE(result->as_int64(), 42);  // copy sollte den Wert behalten, nicht die Referenz
}

QTEST_MAIN(LispQuoteTests)
#include "test_lisp_setq.moc"