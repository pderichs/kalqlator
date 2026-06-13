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

#include "../src/model/Cell.h"
#include "../src/model/Document.h"
#include "TestBase.h"

// Full-stack integration test that exercises the complete calculation flow
// through the public Document API (the same entry point the UI / view model
// use): user input -> Sheet::update_cell -> tokenizer -> parser -> evaluator
// -> TableLispEnvironment -> Cell::visible_content_.
class DocumentIntegrationTests : public lisp::TestBase {
  Q_OBJECT

private:
  // Cell coordinates are (row, column), both zero-based.
  static constexpr int kA1Row = 0;
  static constexpr int kA1Col = 0;
  static constexpr int kB4Row = 3;
  static constexpr int kB4Col = 1;
  static constexpr int kC19Row = 18;
  static constexpr int kC19Col = 2;

private slots:
  // Simulates entering "2" into A1, "6" into B4 and "=(+ A1 B4)" into C19,
  // then verifies that C19 evaluates to "8".
  static void formulaSumsReferencedCells();
};

void DocumentIntegrationTests::formulaSumsReferencedCells() {
  Document document;
  document.initialize();

  // Simulate the user typing into the individual cells.
  document.set_cell_content(kA1Row, kA1Col, "2");
  document.set_cell_content(kB4Row, kB4Col, "6");
  document.set_cell_content(kC19Row, kC19Col, "=(+ A1 B4)");

  // Sanity check: the plain numeric inputs are shown verbatim.
  const Cell *a1 = document.get_cell(kA1Row, kA1Col);
  const Cell *b4 = document.get_cell(kB4Row, kB4Col);
  QVERIFY2(a1 != nullptr, "A1 must exist after input");
  QVERIFY2(b4 != nullptr, "B4 must exist after input");
  QCOMPARE(a1->visible_content_, std::string("2"));
  QCOMPARE(b4->visible_content_, std::string("6"));

  // The formula cell must hold the evaluated result.
  const Cell *c19 = document.get_cell(kC19Row, kC19Col);
  QVERIFY2(c19 != nullptr, "C19 must exist after input");
  QVERIFY2(!c19->has_errors(), "C19 must evaluate without errors");
  QCOMPARE(c19->visible_content_, std::string("8"));
}

QTEST_MAIN(DocumentIntegrationTests)
#include "test_document_integration.moc"
