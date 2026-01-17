// KalQlator - test_lisp_evaluator.cpp
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
#include "../src/lisp/DefaultEnvironment.h"
#include "../src/lisp/Evaluator.h"
#include "../src/lisp/factories.h"
#include "../src/tools/tools.h"

using namespace lisp;

class LispEvaluatorTests : public TestBase {
    Q_OBJECT

private slots:
    void simple_int_atom1();

    void defined_symbol1();
    void defined_symbol2();
    void defined_symbol3();

    void addition1();
    void addition2();
    void addition3();
    void addition4();

    void subtraction1();

    void subtraction2();

    void subtraction3();

    void subtraction4();

    void multiplication1();

    void multiplication2();

    void division1();

    void division2();

    void adding_nils_1();
    void adding_nils_2();
};

void LispEvaluatorTests::simple_int_atom1() {
    LispObjectPtrVector lisp = parseAllString("42");

    EnvironmentPtr env = std::make_shared<Environment>(nullptr);
    Evaluator evaluator(env, {});

    auto result = evaluator.evaluate(lisp);

    QVERIFY(result->is_integer());
    QCOMPARE(result->as_int64(), 42);
}

void LispEvaluatorTests::defined_symbol1() {
    LispObjectPtrVector lisp = parseAllString("foo");

    EnvironmentPtr env = std::make_shared<Environment>(nullptr);
    env->define("foo", make_int(14));
    Evaluator evaluator(env, {});

    auto result = evaluator.evaluate(lisp);

    QVERIFY(result->is_integer());
    QCOMPARE(result->as_int64(), 14);
}

void LispEvaluatorTests::defined_symbol2() {
    // Parent env will be used if symbol cannot be found
    LispObjectPtrVector lisp = parseAllString("foo");

    EnvironmentPtr env = std::make_shared<Environment>(nullptr);
    env->define("foo", make_int(1981));

    EnvironmentPtr env2 = std::make_shared<Environment>(env);
    Evaluator evaluator(env2, {});

    auto result = evaluator.evaluate(lisp);

    QVERIFY(result->is_integer());
    QCOMPARE(result->as_int64(), 1981);
}

void LispEvaluatorTests::defined_symbol3() {
    // Sub env can override parent env
    LispObjectPtrVector lisp = parseAllString("foo");

    EnvironmentPtr env = std::make_shared<Environment>(nullptr);
    env->define("foo", make_int(1981));

    EnvironmentPtr env2 = std::make_shared<Environment>(env);
    env2->define("foo", make_int(1987));
    Evaluator evaluator(env2, {});

    auto result = evaluator.evaluate(lisp);

    QVERIFY(result->is_integer());
    QCOMPARE(result->as_int64(), 1987);
}

void LispEvaluatorTests::addition1() {
    LispObjectPtrVector lisp = parseAllString("(+ 42 5)");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_integer());
    QCOMPARE(result->as_int64(), 47);
}

void LispEvaluatorTests::addition2() {
    LispObjectPtrVector lisp = parseAllString("(+ 42 5 200 1000 -2)");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_integer());
    QCOMPARE(result->as_int64(), 1245);
}

void LispEvaluatorTests::addition3() {
    LispObjectPtrVector lisp = parseAllString("(+ 3)");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_integer());
    QCOMPARE(result->as_int64(), 3);
}

void LispEvaluatorTests::addition4() {
    LispObjectPtrVector lisp = parseAllString("(+ 3 (+ 1 (+ 66 1)))");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_integer());
    QCOMPARE(result->as_int64(), 71);
}

void LispEvaluatorTests::subtraction1() {
    LispObjectPtrVector lisp = parseAllString("(- 42 5)");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_integer());
    QCOMPARE(result->as_int64(), 37);
}

void LispEvaluatorTests::subtraction2() {
    LispObjectPtrVector lisp = parseAllString("(- 42 5 2 1 -2)");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_integer());
    QCOMPARE(result->as_int64(), 36);
}

void LispEvaluatorTests::subtraction3() {
    LispObjectPtrVector lisp = parseAllString("(- 3)");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_integer());
    QCOMPARE(result->as_int64(), 3);
}

void LispEvaluatorTests::subtraction4() {
    LispObjectPtrVector lisp = parseAllString("(- 3 (- 1 (- 66 1)))");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_integer());
    QCOMPARE(result->as_int64(), 67);
}

void LispEvaluatorTests::multiplication1() {
    LispObjectPtrVector lisp = parseAllString("(* 4 2)");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_integer());
    QCOMPARE(result->as_int64(), 8);
}

void LispEvaluatorTests::multiplication2() {
    LispObjectPtrVector lisp = parseAllString("(* 4 2 (* 5 10))");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_integer());
    QCOMPARE(result->as_int64(), 400);
}

void LispEvaluatorTests::division1() {
    LispObjectPtrVector lisp = parseAllString("(/ 10 5)");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_integer());
    QCOMPARE(result->as_int64(), 2);
}

void LispEvaluatorTests::division2() {
    LispObjectPtrVector lisp = parseAllString("(/ 100 5 (/ 2 1))");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_integer());
    QCOMPARE(result->as_int64(), 10);
}

void LispEvaluatorTests::adding_nils_1() {
    LispObjectPtrVector lisp = parseAllString("(+ nil nil)");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_integer());
    QCOMPARE(result->as_int64(), 0);
}

void LispEvaluatorTests::adding_nils_2() {
    LispObjectPtrVector lisp = parseAllString("(+ nil 2)");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_integer());
    QCOMPARE(result->as_int64(), 2);
}

QTEST_MAIN(LispEvaluatorTests)
#include "test_lisp_evaluator.moc"
