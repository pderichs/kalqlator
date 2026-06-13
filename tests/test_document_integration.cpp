// KalQlator - test_document_integration.cpp
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

#include "../src/messagebus/event_dispatcher.h"
#include "../src/messagebus/event_sink.h"
#include "../src/model/Cell.h"
#include "../src/model/CellErrorType.h"
#include "../src/model/Document.h"
#include "../src/model/events/TableEnvironmentUpdateEvent.h"
#include "TestBase.h"

namespace {
// Minimal EventSink that mirrors what MainWindow does for cell recalculation:
// whenever the table environment signals an update it refreshes the dependent
// cells. This lets the headless test exercise the reactive recalculation path
// that, in the running application, is wired through the message bus.
//
// Registration is handled via RAII so the static dispatcher pointer is always
// cleared before this stack object is destroyed, on every exit path.
class RecalculatingSink : public EventSink {
public:
  explicit RecalculatingSink(Document *document) : document_(document) {
    EventDispatcher::registerSink(this);
  }

  ~RecalculatingSink() override { EventDispatcher::registerSink(nullptr); }

  RecalculatingSink(const RecalculatingSink &) = delete;
  RecalculatingSink &operator=(const RecalculatingSink &) = delete;
  RecalculatingSink(RecalculatingSink &&) = delete;
  RecalculatingSink &operator=(RecalculatingSink &&) = delete;

  void onEvent(const std::string &name, const std::any &payload) override {
    if (name == TableEnvironmentUpdateEvent::event_name) {
      const auto &event =
          std::any_cast<const TableEnvironmentUpdateEvent &>(payload);
      document_->refresh_cells(event.name, event.value,
                               event.dependencies_in_topological_order);
    }
  }

private:
  Document *document_;
};
} // namespace

// Full-stack integration tests that exercise the complete calculation flow
// through the public Document API (the same entry point the UI / view model
// use): user input -> Sheet::update_cell -> tokenizer -> parser -> evaluator
// -> TableLispEnvironment -> Cell.
class DocumentIntegrationTests : public lisp::TestBase {
  Q_OBJECT

private:
  // Cell coordinates are (row, column), both zero-based.
  static constexpr int kA1Row = 0;
  static constexpr int kA1Col = 0;
  static constexpr int kA2Row = 1;
  static constexpr int kA2Col = 0;
  static constexpr int kB4Row = 3;
  static constexpr int kB4Col = 1;
  static constexpr int kC19Row = 18;
  static constexpr int kC19Col = 2;

private slots:
  // Make sure no sink leaks from one test into the next.
  static void cleanup() { EventDispatcher::registerSink(nullptr); }

  // Simulates entering "2" into A1, "6" into B4 and "=(+ A1 B4)" into C19,
  // then verifies that C19 evaluates to "8".
  static void formulaSumsReferencedCells();

  // A1 -> A2 and A2 -> A1 must be rejected with a circular reference error
  // and the offending cell must carry the ERROR_CIRCREF status.
  static void circularReferenceProducesCircRefError();

  // A formula that cannot be evaluated (here: an unknown function) must mark
  // the cell with ERROR_GENERAL instead of letting the exception escape.
  static void invalidFormulaProducesGeneralError();

  // After a cell has an error, entering a valid value must clear the error.
  static void errorIsClearedAfterValidReentry();

  // Changing a source cell must propagate to dependent formula cells via the
  // table environment update event (reactive recalculation).
  static void dependentCellRecalculatesOnSourceChange();
};

void DocumentIntegrationTests::formulaSumsReferencedCells() {
  Document document;
  document.initialize();

  // Simulate the user typing into the individual cells.
  document.set_cell_content(kA1Row, kA1Col, "2");
  document.set_cell_content(kB4Row, kB4Col, "6");
  document.set_cell_content(kC19Row, kC19Col, "=(+ A1 B4)");

  // Sanity check: the plain numeric inputs are shown verbatim.
  const Cell *cell_a1 = document.get_cell(kA1Row, kA1Col);
  const Cell *cell_b4 = document.get_cell(kB4Row, kB4Col);
  if (cell_a1 == nullptr || cell_b4 == nullptr) {
    QFAIL("A1 and B4 must exist after input");
  }
  QCOMPARE(cell_a1->visible_content_, std::string("2"));
  QCOMPARE(cell_b4->visible_content_, std::string("6"));

  // The formula cell must hold the evaluated result.
  const Cell *cell_c19 = document.get_cell(kC19Row, kC19Col);
  if (cell_c19 == nullptr) {
    QFAIL("C19 must exist after input");
  }
  QVERIFY2(!cell_c19->has_errors(), "C19 must evaluate without errors");
  QCOMPARE(cell_c19->visible_content_, std::string("8"));
}

void DocumentIntegrationTests::circularReferenceProducesCircRefError() {
  Document document;
  document.initialize();

  // A1 references A2 (A2 is still empty -> evaluates fine).
  document.set_cell_content(kA1Row, kA1Col, "=(+ A2 1)");
  const Cell *cell_a1 = document.get_cell(kA1Row, kA1Col);
  if (cell_a1 == nullptr) {
    QFAIL("A1 must exist after input");
  }
  QVERIFY2(!cell_a1->has_errors(), "A1 alone must not be circular");

  // A2 now references A1 -> this closes the cycle and must be rejected.
  document.set_cell_content(kA2Row, kA2Col, "=(+ A1 1)");
  const Cell *cell_a2 = document.get_cell(kA2Row, kA2Col);
  if (cell_a2 == nullptr) {
    QFAIL("A2 must exist after input");
  }
  QVERIFY2(cell_a2->has_errors(),
           "A2 must be flagged when it closes the cycle");

  const auto error = cell_a2->get_last_error();
  QVERIFY2(error.has_value(), "A2 must carry an error");
  QCOMPARE(error->error_type, ERROR_CIRCREF);

  // The rejected formula must not be kept as an active formula.
  QVERIFY2(cell_a2->raw_formula_.empty(),
           "a rejected circular formula must not stay active");
}

void DocumentIntegrationTests::invalidFormulaProducesGeneralError() {
  Document document;
  document.initialize();

  document.set_cell_content(kA1Row, kA1Col, "=(nonexistentfunction 1 2)");

  const Cell *cell_a1 = document.get_cell(kA1Row, kA1Col);
  if (cell_a1 == nullptr) {
    QFAIL("A1 must exist after input");
  }
  QVERIFY2(cell_a1->has_errors(), "an unevaluable formula must flag the cell");

  const auto error = cell_a1->get_last_error();
  QVERIFY2(error.has_value(), "A1 must carry an error");
  QCOMPARE(error->error_type, ERROR_GENERAL);
}

void DocumentIntegrationTests::errorIsClearedAfterValidReentry() {
  Document document;
  document.initialize();

  // First produce an error ...
  document.set_cell_content(kA1Row, kA1Col, "=(nonexistentfunction 1 2)");
  const Cell *cell_a1 = document.get_cell(kA1Row, kA1Col);
  if (cell_a1 == nullptr) {
    QFAIL("A1 must exist after input");
  }
  QVERIFY2(cell_a1->has_errors(), "precondition: A1 must have an error");

  // ... then overwrite it with a valid value.
  document.set_cell_content(kA1Row, kA1Col, "42");
  QVERIFY2(!cell_a1->has_errors(),
           "a valid re-entry must clear previous errors");
  QCOMPARE(cell_a1->visible_content_, std::string("42"));
}

void DocumentIntegrationTests::dependentCellRecalculatesOnSourceChange() {
  Document document;
  document.initialize();

  // Wire up the reactive recalculation the same way MainWindow does.
  // The sink registers itself with the dispatcher and unregisters on
  // destruction (RAII).
  const RecalculatingSink sink(&document);

  document.set_cell_content(kA1Row, kA1Col, "2");
  document.set_cell_content(kB4Row, kB4Col, "6");
  document.set_cell_content(kC19Row, kC19Col, "=(+ A1 B4)");

  const Cell *cell_c19 = document.get_cell(kC19Row, kC19Col);
  if (cell_c19 == nullptr) {
    QFAIL("C19 must exist after input");
  }
  QCOMPARE(cell_c19->visible_content_, std::string("8"));

  // Changing A1 must recalculate the dependent formula in C19.
  document.set_cell_content(kA1Row, kA1Col, "10");
  QCOMPARE(cell_c19->visible_content_, std::string("16"));
}

QTEST_MAIN(DocumentIntegrationTests)
#include "test_document_integration.moc"
