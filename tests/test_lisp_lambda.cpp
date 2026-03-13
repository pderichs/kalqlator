// KalQlator - test_lisp_lambda.cpp
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
#include "../src/tools/tools.h"
#include "../src/lisp/DefaultEnvironment.h"
#include "../src/lisp/tools.h"

using namespace lisp;

class LispLambdaTests : public TestBase {
    Q_OBJECT

private slots:
    // Basis-Tests
    static void simple_lambda();

    static void lambda_with_single_parameter();

    static void lambda_with_multiple_parameters();

    // Rückgabewerte
    static void lambda_returns_number();

    static void lambda_returns_list();

    static void lambda_returns_nil();

    // Body mit mehreren Expressions
    static void lambda_with_multiple_body_expressions();

    // Closures
    static void lambda_captures_outer_variable();

    static void lambda_closure_preserves_value();

    static void nested_lambda();

    // Higher-Order Functions
    static void lambda_as_argument();

    static void lambda_returns_lambda();

    // Rekursion
    static void recursive_lambda_with_define();

    // Arithmetik im Lambda
    static void lambda_with_arithmetic();

    // Conditionals im Lambda
    static void lambda_with_conditional();

    // Fehlerbehandlung
    static void lambda_wrong_argument_count();
};

void LispLambdaTests::simple_lambda() {
    LispObjectPtrVector lisp = parse_all_string("(define hello (lambda () \"Hello\")) (setq a (hello)) a");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_string());
    QCOMPARE(result->as_string(), "Hello");
}

void LispLambdaTests::lambda_with_single_parameter() {
    LispObjectPtrVector lisp = parse_all_string("(define double (lambda (x) (* x 2))) (double 5)");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_number());
    QCOMPARE(result->as_int64(), 10);
}

void LispLambdaTests::lambda_with_multiple_parameters() {
    LispObjectPtrVector lisp = parse_all_string("(define add (lambda (a b c) (+ a b c))) (add 1 2 3)");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_number());
    QCOMPARE(result->as_int64(), 6);
}

void LispLambdaTests::lambda_returns_number() {
    LispObjectPtrVector lisp = parse_all_string("(define answer (lambda () 42)) (answer)");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_number());
    QCOMPARE(result->as_int64(), 42);
}

void LispLambdaTests::lambda_returns_list() {
    LispObjectPtrVector lisp = parse_all_string("(define make-pair (lambda (a b) (list a b))) (make-pair 1 2)");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_list());
    QCOMPARE(countListElements(result), 2);
}

void LispLambdaTests::lambda_returns_nil() {
    LispObjectPtrVector lisp = parse_all_string("(define nothing (lambda () nil)) (nothing)");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_nil());
}

void LispLambdaTests::lambda_with_multiple_body_expressions() {
    // Der letzte Ausdruck im Body ist der Rückgabewert
    LispObjectPtrVector lisp = parse_all_string(R"(
        (define side-effect-fn (lambda (x)
            (setq temp x)
            (+ temp 10)))
        (side-effect-fn 5)
    )");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_number());
    QCOMPARE(result->as_int64(), 15);
}

void LispLambdaTests::lambda_captures_outer_variable() {
    LispObjectPtrVector lisp = parse_all_string(R"(
        (setq multiplier 10)
        (define scale (lambda (x) (* x multiplier)))
        (scale 5)
    )");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_number());
    QCOMPARE(result->as_int64(), 50);
}

void LispLambdaTests::lambda_closure_preserves_value() {
    // Closure sollte den Wert zum Zeitpunkt der Definition bewahren
    LispObjectPtrVector lisp = parse_all_string(R"(
        (setq x 10)
        (define get-x (lambda () x))
        (setq x 20)
        (get-x)
    )");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_number());
    // Je nach Implementierung: 10 (lexical scope) oder 20 (dynamic scope)
    // Anpassen je nach deiner Semantik
    QCOMPARE(result->as_int64(), 20);
}

void LispLambdaTests::nested_lambda() {
    LispObjectPtrVector lisp = parse_all_string(R"(
        (define outer (lambda (x)
            (define inner (lambda (y) (+ x y)))
            (inner 5)))
        (outer 10)
    )");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_number());
    QCOMPARE(result->as_int64(), 15);
}

void LispLambdaTests::lambda_as_argument() {
    LispObjectPtrVector lisp = parse_all_string(R"(
        (define apply-twice (lambda (f x) (f (f x))))
        (define increment (lambda (n) (+ n 1)))
        (apply-twice increment 5)
    )");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_number());
    QCOMPARE(result->as_int64(), 7);
}

void LispLambdaTests::lambda_returns_lambda() {
    LispObjectPtrVector lisp = parse_all_string(R"(
        (define make-adder (lambda (n)
            (lambda (x) (+ x n))))
        (define add5 (make-adder 5))
        (add5 10)
    )");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_number());
    QCOMPARE(result->as_int64(), 15);
}

void LispLambdaTests::recursive_lambda_with_define() {
    LispObjectPtrVector lisp = parse_all_string(R"(
        (define factorial (lambda (n)
            (if (<= n 1)
                1
                (* n (factorial (- n 1))))))
        (factorial 5)
    )");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_number());
    QCOMPARE(result->as_int64(), 120);
}

void LispLambdaTests::lambda_with_arithmetic() {
    LispObjectPtrVector lisp = parse_all_string(R"(
        (define quadratic (lambda (a b c x)
            (+ (* a (* x x)) (* b x) c)))
        (quadratic 1 2 1 3)
    )");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_number());
    // 1*3² + 2*3 + 1 = 9 + 6 + 1 = 16
    QCOMPARE(result->as_int64(), 16);
}

void LispLambdaTests::lambda_with_conditional() {
    LispObjectPtrVector lisp = parse_all_string(R"(
        (define abs (lambda (x)
            (if (< x 0) (- 0 x) x)))
        (abs -42)
    )");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_number());
    QCOMPARE(result->as_int64(), 42);
}

void LispLambdaTests::lambda_wrong_argument_count() {
    LispObjectPtrVector lisp = parse_all_string(R"(
        (define needs-two (lambda (a b) (+ a b)))
        (needs-two 1)
    )");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    // Je nach Implementierung: Exception oder Error-Objekt
    QVERIFY_EXCEPTION_THROWN(evaluator.evaluate(lisp), std::exception);
}

QTEST_MAIN(LispLambdaTests)
#include "test_lisp_lambda.moc"