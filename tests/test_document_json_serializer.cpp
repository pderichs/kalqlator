// KalQlator - test_document_json_serializer.cpp
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

#include <QTemporaryDir>

#include <memory>

#include "../src/file/DocumentJsonSerializer.h"

class DocumentJsonSerializerTests : public QObject {
  Q_OBJECT

private:
  static constexpr int kA1Row = 0;
  static constexpr int kA1Col = 0;
  static constexpr int kA2Row = 1;
  static constexpr int kA2Col = 0;
  static constexpr int kB4Row = 3;
  static constexpr int kB4Col = 1;
  static constexpr int kC19Row = 18;
  static constexpr int kC19Col = 2;

  static void updateAllSheets(Document *document) {
    for (size_t i = 0; i < document->sheet_count(); ++i) {
      document->set_active_sheet(i);
      document->update_all_cells();
    }
  }

private slots:
  static void roundTripPreservesSheetsAndPerSheetState();

  static void failedOpenLeavesExistingDocumentUntouched();
};

void DocumentJsonSerializerTests::roundTripPreservesSheetsAndPerSheetState() {
  auto document = std::make_shared<Document>();
  document->initialize();

  document->set_cell_content(kA1Row, kA1Col, "5");
  document->set_cell_content(kC19Row, kC19Col, "=(+ A1 1)");
  document->set_current_cell(Location(kC19Col, kC19Row));
  document->set_selected_cells(
      LocationSet{Location(kA1Col, kA1Row), Location(kC19Col, kC19Row)});
  document->set_row_height(18, 33);
  document->set_column_width(2, 77);

  const int second_sheet_index = document->add_next_sheet();
  QCOMPARE(second_sheet_index, 1);
  document->set_active_sheet(1);
  document->set_cell_content(kB4Row, kB4Col, "second sheet");
  document->set_cell_content(kA2Row, kA2Col, "200");
  document->set_current_cell(Location(kB4Col, kB4Row));
  document->set_selected_cells(LocationSet{Location(kB4Col, kB4Row)});
  document->set_row_height(3, 44);
  document->set_column_width(1, 88);

  const int third_sheet_index = document->add_next_sheet();
  QCOMPARE(third_sheet_index, 2);
  document->set_active_sheet(2);
  document->set_cell_content(kA1Row, kA1Col, "third sheet");

  document->set_active_sheet(0);

  QTemporaryDir temp_dir;
  QVERIFY2(temp_dir.isValid(), "temporary directory must be created");
  const auto filename = temp_dir.filePath("document.json").toStdString();

  DocumentJsonSerializer serializer(document, filename);
  QVERIFY2(serializer.save(), "saving the document must succeed");

  auto opened_document = std::make_shared<Document>();
  opened_document->initialize();
  DocumentJsonSerializer opened_serializer(opened_document, filename);
  QVERIFY2(opened_serializer.open(), "opening the saved document must work");

  QCOMPARE(opened_document->sheet_count(), size_t{3});
  QCOMPARE(opened_document->get_active_sheet(), size_t{0});

  const auto *const first_sheet = opened_document->sheet_by_index(0);
  const auto *const second_sheet = opened_document->sheet_by_index(1);
  const auto *const third_sheet = opened_document->sheet_by_index(2);
  if (first_sheet == nullptr || second_sheet == nullptr ||
      third_sheet == nullptr) {
    QFAIL("all sheets must exist after opening the document");
  }

  const auto *const first_a1 = first_sheet->get_cell(kA1Row, kA1Col);
  const auto *const first_c19 = first_sheet->get_cell(kC19Row, kC19Col);
  const auto *const second_b4 = second_sheet->get_cell(kB4Row, kB4Col);
  const auto *const second_a2 = second_sheet->get_cell(kA2Row, kA2Col);
  if (first_a1 == nullptr || first_c19 == nullptr || second_b4 == nullptr ||
      second_a2 == nullptr) {
    QFAIL("expected cells must exist after opening the document");
  }

  QCOMPARE(first_a1->raw_content_, std::string("5"));
  QCOMPARE(first_c19->raw_content_, std::string("=(+ A1 1)"));
  QCOMPARE(second_b4->raw_content_, std::string("second sheet"));
  QCOMPARE(second_a2->raw_content_, std::string("200"));
  const auto *const third_a1 = third_sheet->get_cell(kA1Row, kA1Col);
  if (third_a1 == nullptr) {
    QFAIL(
        "the expected third-sheet cell must exist after opening the document");
  }
  QCOMPARE(third_a1->raw_content_, std::string("third sheet"));
  const auto *const second_a1 = second_sheet->get_cell(kA1Row, kA1Col);
  QVERIFY2(second_a1 != nullptr,
           "sheet initialization creates an empty A1 model on every sheet");
  QCOMPARE(second_a1->raw_content_, std::string(""));
  QCOMPARE(second_a1->raw_formula_, std::string(""));
  QVERIFY2(second_sheet->get_cell(kC19Row, kC19Col) == nullptr,
           "sheet 2 must not receive formulas from sheet 1 during open");

  QCOMPARE(first_sheet->get_current_selected_cell(),
           Location(kC19Col, kC19Row));
  QCOMPARE(first_sheet->get_selected_cells().size(), size_t{2});
  QVERIFY(first_sheet->get_selected_cells().contains(Location(kA1Col, kA1Row)));
  QVERIFY(
      first_sheet->get_selected_cells().contains(Location(kC19Col, kC19Row)));
  QCOMPARE(first_sheet->get_row_heights().at(18), size_t{33});
  QCOMPARE(first_sheet->get_column_widths().at(2), size_t{77});

  QCOMPARE(second_sheet->get_current_selected_cell(), Location(kB4Col, kB4Row));
  QCOMPARE(second_sheet->get_selected_cells().size(), size_t{1});
  QVERIFY(
      second_sheet->get_selected_cells().contains(Location(kB4Col, kB4Row)));
  QCOMPARE(second_sheet->get_row_heights().at(3), size_t{44});
  QCOMPARE(second_sheet->get_column_widths().at(1), size_t{88});

  updateAllSheets(opened_document.get());
  QCOMPARE(first_c19->visible_content_, std::string("6"));
  QCOMPARE(second_b4->visible_content_, std::string("second sheet"));
  QCOMPARE(third_a1->visible_content_, std::string("third sheet"));
}

void DocumentJsonSerializerTests::failedOpenLeavesExistingDocumentUntouched() {
  auto document = std::make_shared<Document>();
  document->initialize();
  document->set_cell_content(kA1Row, kA1Col, "42");
  document->set_current_cell(Location(kA1Col, kA1Row));
  const int second_sheet_index = document->add_next_sheet();
  QCOMPARE(second_sheet_index, 1);
  document->set_active_sheet(1);
  document->set_cell_content(kB4Row, kB4Col, "keep me");

  QTemporaryDir temp_dir;
  QVERIFY2(temp_dir.isValid(), "temporary directory must be created");
  const auto missing_filename =
      temp_dir.filePath("does-not-exist.json").toStdString();

  DocumentJsonSerializer serializer(document, missing_filename);
  QVERIFY2(!serializer.open(), "opening a missing file must fail");

  QCOMPARE(document->sheet_count(), size_t{2});

  document->set_active_sheet(0);
  const auto *const first_sheet_cell = document->get_cell(kA1Row, kA1Col);
  if (first_sheet_cell == nullptr) {
    QFAIL("the original first-sheet cell must still exist after failed open");
  }
  QCOMPARE(first_sheet_cell->raw_content_, std::string("42"));

  document->set_active_sheet(1);
  const auto *const second_sheet_cell = document->get_cell(kB4Row, kB4Col);
  if (second_sheet_cell == nullptr) {
    QFAIL("the original second-sheet cell must still exist after failed open");
  }
  QCOMPARE(second_sheet_cell->raw_content_, std::string("keep me"));
}

QTEST_MAIN(DocumentJsonSerializerTests)
#include "test_document_json_serializer.moc"
