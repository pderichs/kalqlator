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

#include "../src/lisp/DefaultEnvironment.h"
#include "../src/lisp/Evaluator.h"
#include "../src/lisp/factories.h"
#include "../src/lisp/tools.h"
#include "../src/tools/tools.h"
#include "TestBase.h"
#include "test_tools.h"

using namespace lisp;

class LispEvaluatorTests : public TestBase {
  Q_OBJECT

private slots:
  static void simple_int_atom1();

  static void defined_symbol1();
  static void defined_symbol2();
  static void defined_symbol3();

  static void addition1();
  static void addition2();
  static void addition3();
  static void addition4();

  static void subtraction1();

  static void subtraction2();

  static void subtraction3();

  static void subtraction4();

  static void multiplication1();

  static void multiplication2();

  static void division1();

  static void division2();

  static void adding_nils_1();
  static void adding_nils_2();
};

void LispEvaluatorTests::simple_int_atom1() {
  LispObjectPtrVector lisp = parse_all_string("42");

  EnvironmentPtr env = std::make_shared<Environment>(nullptr);
  Evaluator evaluator(env, {});

  auto result = evaluator.evaluate(lisp);

  QVERIFY(result->is_number());
  QCOMPARE(result->as_number(), mpq_class(42));
}

void LispEvaluatorTests::defined_symbol1() {
  LispObjectPtrVector lisp = parse_all_string("foo");

  EnvironmentPtr env = std::make_shared<Environment>(nullptr);
  env->define("foo", make_number(14));
  Evaluator evaluator(env, {});

  auto result = evaluator.evaluate(lisp);

  QVERIFY(result->is_number());
  QCOMPARE(result->as_number(), mpq_class(14));
}

void LispEvaluatorTests::defined_symbol2() {
  // Parent env will be used if symbol cannot be found
  LispObjectPtrVector lisp = parse_all_string("foo");

  EnvironmentPtr env = std::make_shared<Environment>(nullptr);
  env->define("foo", make_number(1981));

  EnvironmentPtr env2 = std::make_shared<Environment>(env);
  Evaluator evaluator(env2, {});

  auto result = evaluator.evaluate(lisp);

  QVERIFY(result->is_number());
  QCOMPARE(result->as_number(), mpq_class(1981));
}

void LispEvaluatorTests::defined_symbol3() {
  // Sub env can override parent env
  LispObjectPtrVector lisp = parse_all_string("foo");

  EnvironmentPtr env = std::make_shared<Environment>(nullptr);
  env->define("foo", make_number(1981));

  EnvironmentPtr env2 = std::make_shared<Environment>(env);
  env2->define("foo", make_number(1987));
  Evaluator evaluator(env2, {});

  auto result = evaluator.evaluate(lisp);

  QVERIFY(result->is_number());
  QCOMPARE(result->as_number(), mpq_class(1987));
}

void LispEvaluatorTests::addition1() {
  LispObjectPtrVector lisp = parse_all_string("(+ 42 5)");
  EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
  Evaluator evaluator(env, {});
  auto result = evaluator.evaluate(lisp);
  QVERIFY(result->is_number());
  QCOMPARE(result->as_number(), mpq_class(47));
}

void LispEvaluatorTests::addition2() {
  LispObjectPtrVector lisp = parse_all_string("(+ 42 5 200 1000 -2)");
  EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
  Evaluator evaluator(env, {});
  auto result = evaluator.evaluate(lisp);
  QVERIFY(result->is_number());
  QCOMPARE(result->as_number(), mpq_class(1245));
}

void LispEvaluatorTests::addition3() {
  LispObjectPtrVector lisp = parse_all_string("(+ 3)");
  EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
  Evaluator evaluator(env, {});
  auto result = evaluator.evaluate(lisp);
  QVERIFY(result->is_number());
  QCOMPARE(result->as_number(), mpq_class(3));
}

void LispEvaluatorTests::addition4() {
  LispObjectPtrVector lisp = parse_all_string("(+ 3 (+ 1 (+ 66 1)))");
  EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
  Evaluator evaluator(env, {});
  auto result = evaluator.evaluate(lisp);
  QVERIFY(result->is_number());
  QCOMPARE(result->as_number(), mpq_class(71));
}

void LispEvaluatorTests::subtraction1() {
  LispObjectPtrVector lisp = parse_all_string("(- 42 5)");
  EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
  Evaluator evaluator(env, {});
  auto result = evaluator.evaluate(lisp);
  QVERIFY(result->is_number());
  QCOMPARE(result->as_number(), mpq_class(37));
}

void LispEvaluatorTests::subtraction2() {
  LispObjectPtrVector lisp = parse_all_string("(- 42 5 2 1 -2)");
  EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
  Evaluator evaluator(env, {});
  auto result = evaluator.evaluate(lisp);
  QVERIFY(result->is_number());
  QCOMPARE(result->as_number(), mpq_class(36));
}

void LispEvaluatorTests::subtraction3() {
  LispObjectPtrVector lisp = parse_all_string("(- 3)");
  EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
  Evaluator evaluator(env, {});
  auto result = evaluator.evaluate(lisp);
  QVERIFY(result->is_number());
  QCOMPARE(result->as_number(), mpq_class(3));
}

void LispEvaluatorTests::subtraction4() {
  LispObjectPtrVector lisp = parse_all_string("(- 3 (- 1 (- 66 1)))");
  EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
  Evaluator evaluator(env, {});
  auto result = evaluator.evaluate(lisp);
  QVERIFY(result->is_number());
  QCOMPARE(result->as_number(), mpq_class(67));
}

void LispEvaluatorTests::multiplication1() {
  LispObjectPtrVector lisp = parse_all_string("(* 4 2)");
  EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
  Evaluator evaluator(env, {});
  auto result = evaluator.evaluate(lisp);
  QVERIFY(result->is_number());
  QCOMPARE(result->as_number(), mpq_class(8));
}

void LispEvaluatorTests::multiplication2() {
  LispObjectPtrVector lisp = parse_all_string("(* 4 2 (* 5 10))");
  EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
  Evaluator evaluator(env, {});
  auto result = evaluator.evaluate(lisp);
  QVERIFY(result->is_number());
  QCOMPARE(result->as_number(), mpq_class(400));
}

void LispEvaluatorTests::division1() {
  LispObjectPtrVector lisp = parse_all_string("(/ 10 5)");
  EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
  Evaluator evaluator(env, {});
  auto result = evaluator.evaluate(lisp);
  QVERIFY(result->is_number());
  QCOMPARE(result->as_number(), mpq_class(2));
}

void LispEvaluatorTests::division2() {
  LispObjectPtrVector lisp = parse_all_string("(/ 100 5 (/ 2 1))");
  EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
  Evaluator evaluator(env, {});
  auto result = evaluator.evaluate(lisp);
  QVERIFY(result->is_number());
  QCOMPARE(result->as_number(), mpq_class(10));
}

void LispEvaluatorTests::adding_nils_1() {
  LispObjectPtrVector lisp = parse_all_string("(+ nil nil)");
  EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
  Evaluator evaluator(env, {});
  auto result = evaluator.evaluate(lisp);
  QVERIFY(result->is_number());
  QCOMPARE(result->as_number(), mpq_class(0));
}

void LispEvaluatorTests::adding_nils_2() {
  LispObjectPtrVector lisp = parse_all_string("(+ nil 2)");
  EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
  Evaluator evaluator(env, {});
  auto result = evaluator.evaluate(lisp);
  QVERIFY(result->is_number());
  QCOMPARE(result->as_number(), mpq_class(2));
}

QTEST_MAIN(LispEvaluatorTests)
#include "test_lisp_evaluator.moc"
