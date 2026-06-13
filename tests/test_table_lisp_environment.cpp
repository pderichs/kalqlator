// KalQlator - test_table_lisp_environment.cpp
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

#include <any>
#include <memory>

#include "../src/lisp/Evaluator.h"
#include "../src/lisp/factories.h"
#include "../src/lisp/tools.h"
#include "../src/model/CircularReferenceError.h"
#include "../src/model/TableContext.h"
#include "../src/model/TableLispEnvironment.h"
#include "../src/tools/tools.h"
#include "TestBase.h"

using namespace lisp;

class TableLispEnvironmentTests : public lisp::TestBase {
  Q_OBJECT

private:
  static TableLispEnvironmentPtr makeEnv() {
    auto env = std::make_shared<TableLispEnvironment>();
    env->initialize();
    return env;
  }

  /**
   * Mimics what Sheet::update_cell does for a single cell: clear the old
   * references, evaluate the formula with the cell set as source in the
   * TableContext, then store the resulting value back into the environment.
   *
   * The thrown exceptions (e.g. CircularReferenceError) are intentionally not
   * caught here so the individual tests can assert on them.
   */
  static LispObjectPtr evalInCell(const TableLispEnvironmentPtr &env,
                                  const std::string &cell,
                                  const std::string &formula) {
    env->remove_references(cell);
    LispObjectPtrVector lisp = parse_all_string(formula);
    Evaluator evaluator(
        env, TableContext{.source_cell = cell, .sheet_registry = nullptr});
    auto result = evaluator.evaluate(lisp);
    env->define(cell, result);
    return result;
  }

private slots:
  // --- Core feature: bare cell symbols are reactive references ------------

  // Baseline: "=(+ A1 A2)" must establish A3 -> {A1, A2} in both directions.
  static void bareSymbolsCreateReferences();

  // --- Regression cases that the current changeset must satisfy -----------

  // Critical: function/operator names resolved during evaluation must NOT be
  // tracked as cell references. Otherwise Sheet::collect_reference_cells feeds
  // them into get_cell_location_by_name(), which throws on "+", "cell", ...
  static void functionNamesAreNotTracked();

  // The "(cell X)" marker must keep working for backwards compatibility with
  // already persisted documents, and the symbol "cell" must not be tracked.
  static void cellFunctionRemainsBackwardCompatible();

  // Quoted data is not evaluated, so quoted cell-like symbols must not create
  // dependencies.
  static void quotedSymbolsAreNotTracked();

  // A locally bound symbol (lambda parameter) that happens to be named like a
  // cell must not create a dependency on the actual cell.
  static void lambdaParameterShadowingIsNotTracked();

  // Robustness: lookup must tolerate being called without a TableContext
  // instead of throwing std::bad_any_cast.
  static void lookupWithoutTableContextDoesNotThrowBadAnyCast();

  // --- Behaviour that must keep working -----------------------------------

  // Cyclic dependencies introduced via bare symbols must still be detected.
  static void circularReferenceIsDetected();

  // Documents the known trade-off of dynamic tracking: only the evaluated
  // branch of an "if" is tracked.
  static void onlyEvaluatedIfBranchIsTracked();
};

void TableLispEnvironmentTests::bareSymbolsCreateReferences() {
  auto env = makeEnv();
  evalInCell(env, "A1", "10");
  evalInCell(env, "A2", "20");

  auto result = evalInCell(env, "A3", "(+ A1 A2)");

  QVERIFY(result->is_number());
  QCOMPARE(result->as_number(), mpq_class(30));

  const auto refs = env->get_references("A3");
  QVERIFY2(refs.contains("A1"), "A3 must reference A1");
  QVERIFY2(refs.contains("A2"), "A3 must reference A2");

  QVERIFY2(env->get_referenced_by("A1").contains("A3"),
           "A1 must know it is referenced by A3");
  QVERIFY2(env->get_referenced_by("A2").contains("A3"),
           "A2 must know it is referenced by A3");
}

void TableLispEnvironmentTests::functionNamesAreNotTracked() {
  auto env = makeEnv();
  evalInCell(env, "A1", "10");
  evalInCell(env, "A2", "20");

  evalInCell(env, "A3", "(+ A1 A2)");

  const auto refs = env->get_references("A3");
  QVERIFY2(!refs.contains("+"),
           "operator '+' must not be tracked as a cell reference");
  // Only the two real cells may end up in the dependency graph.
  QVERIFY2(refs.size() == 2,
           "only A1 and A2 may be tracked, no function names");
}

void TableLispEnvironmentTests::cellFunctionRemainsBackwardCompatible() {
  auto env = makeEnv();
  evalInCell(env, "A1", "42");

  auto result = evalInCell(env, "A3", "(cell A1)");

  QVERIFY(result->is_number());
  QCOMPARE(result->as_number(), mpq_class(42));

  const auto refs = env->get_references("A3");
  QVERIFY2(refs.contains("A1"), "(cell A1) must reference A1");
  QVERIFY2(!refs.contains("cell"),
           "the 'cell' function name must not be tracked as a reference");
  QVERIFY2(refs.size() == 1, "only A1 may be tracked");
}

void TableLispEnvironmentTests::quotedSymbolsAreNotTracked() {
  auto env = makeEnv();
  evalInCell(env, "A1", "5");

  evalInCell(env, "A3", "(quote A1)");

  const auto refs = env->get_references("A3");
  QVERIFY2(!refs.contains("A1"), "quoted symbols must not create dependencies");
  QVERIFY2(refs.empty(), "a quoted expression must not track any reference");
}

void TableLispEnvironmentTests::lambdaParameterShadowingIsNotTracked() {
  auto env = makeEnv();

  auto result = evalInCell(env, "A3", "((lambda (A1) A1) 5)");

  QVERIFY(result->is_number());
  QCOMPARE(result->as_number(), mpq_class(5));

  const auto refs = env->get_references("A3");
  QVERIFY2(!refs.contains("A1"),
           "a lambda-bound symbol must not create a cell dependency");
  QVERIFY2(refs.empty(), "no reference must be tracked for a shadowed symbol");
}

void TableLispEnvironmentTests::
    lookupWithoutTableContextDoesNotThrowBadAnyCast() {
  auto env = makeEnv();

  bool threw_bad_any_cast = false;
  try {
    // We only care about the call not throwing bad_any_cast here, so the
    // looked-up value is intentionally discarded.
    static_cast<void>(env->lookup("A1", std::any{}));
  } catch (const std::bad_any_cast &) {
    threw_bad_any_cast = true;
  } catch (...) {
    // Any other outcome (incl. returning a value) is acceptable here.
  }

  QVERIFY2(!threw_bad_any_cast,
           "lookup must tolerate a missing/foreign context instead of "
           "throwing std::bad_any_cast");
}

void TableLispEnvironmentTests::circularReferenceIsDetected() {
  auto env = makeEnv();

  // A1 references A2 (A2 is still nil -> evaluates fine).
  evalInCell(env, "A1", "(+ A2 1)");

  // A2 now references A1 -> this closes the cycle and must be rejected.
  bool threw = false;
  try {
    evalInCell(env, "A2", "(+ A1 1)");
  } catch (const CircularReferenceError &) {
    threw = true;
  }

  QVERIFY2(threw, "a circular reference A1 <-> A2 must be detected");
}

void TableLispEnvironmentTests::onlyEvaluatedIfBranchIsTracked() {
  auto env = makeEnv();
  evalInCell(env, "A1", "5");
  evalInCell(env, "A2", "7");

  auto result = evalInCell(env, "A3", "(if t A1 A2)");

  QVERIFY(result->is_number());
  QCOMPARE(result->as_number(), mpq_class(5));

  const auto refs = env->get_references("A3");
  QVERIFY2(refs.contains("A1"), "the taken branch (A1) must be tracked");
  QVERIFY2(!refs.contains("A2"),
           "the untaken branch (A2) is not tracked (known trade-off of "
           "dynamic dependency tracking)");
}

QTEST_MAIN(TableLispEnvironmentTests)
#include "test_table_lisp_environment.moc"
