// KalQlator - test_clipboard_cells_mime.cpp
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

#include <QMimeData>

#include "../src/ui/ClipboardCellsMime.h"

class ClipboardCellsMimeTests : public QObject {
  Q_OBJECT

private slots:
  static void parseRejectsMissingMimeType();
  static void parseRejectsInvalidCellCount();
  static void parseAcceptsCustomMimeEvenWithTextPlain();
  static void parseRejectsInvalidRoundingMode();
  static void parseRejectsInvalidVersion();
  static void parseRejectsLowerVersion();
  static void parseRejectsMissingVersion();
};

void ClipboardCellsMimeTests::parseRejectsMissingMimeType() {
  QMimeData mime_data;
  mime_data.setText(QStringLiteral("plain-only"));

  const auto payload = parse_clipboard_cells_payload(&mime_data);
  QVERIFY2(!payload.has_value(), "missing custom mime type must be rejected");
}

void ClipboardCellsMimeTests::parseRejectsInvalidCellCount() {
  ClipboardCellsPayload payload;
  payload.rows = 2;
  payload.cols = 2;
  payload.cells = {
      ClipboardCellData{.content = "A"},
      ClipboardCellData{.content = "B"},
  };

  QMimeData mime_data;
  mime_data.setData(KALQLATOR_CLIPBOARD_MIME_TYPE,
                    serialize_clipboard_cells_payload(payload));

  const auto parsed = parse_clipboard_cells_payload(&mime_data);
  QVERIFY2(!parsed.has_value(), "rows*cols mismatch must be rejected");
}

void ClipboardCellsMimeTests::parseAcceptsCustomMimeEvenWithTextPlain() {
  ClipboardCellsPayload payload;
  payload.rows = 1;
  payload.cols = 1;

  CellFormat format;
  format.specifier = "###.## EUR";
  format.rounding_mode = RoundingMode::Nearest;
  format.word_wrap = true;

  payload.cells = {
      ClipboardCellData{.content = "42.5678", .format = format},
  };

  QMimeData mime_data;
  mime_data.setText(QStringLiteral("999"));
  mime_data.setData(KALQLATOR_CLIPBOARD_MIME_TYPE,
                    serialize_clipboard_cells_payload(payload));

  const auto parsed = parse_clipboard_cells_payload(&mime_data);
  QVERIFY2(parsed.has_value(), "custom mime must be parsed when present");
  QCOMPARE(parsed->rows, 1);
  QCOMPARE(parsed->cols, 1);
  QCOMPARE(parsed->cells.size(), size_t{1});
  QCOMPARE(parsed->cells[0].content, std::string("42.5678"));

  QVERIFY2(parsed->cells[0].format.has_value(),
           "format in custom mime must survive roundtrip");
  QCOMPARE(parsed->cells[0].format->specifier, std::string("###.## EUR"));
  QCOMPARE(parsed->cells[0].format->rounding_mode, RoundingMode::Nearest);
  QCOMPARE(parsed->cells[0].format->word_wrap, true);
}

void ClipboardCellsMimeTests::parseRejectsInvalidRoundingMode() {
  const QByteArray invalid_payload =
      R"({"version":1,"rows":1,"cols":1,"cells":[{"content":"1","format":{"specifier":"###.##","rounding_mode":99,"word_wrap":false}}]})";

  QMimeData mime_data;
  mime_data.setData(KALQLATOR_CLIPBOARD_MIME_TYPE, invalid_payload);

  const auto parsed = parse_clipboard_cells_payload(&mime_data);
  QVERIFY2(!parsed.has_value(), "invalid rounding mode must be rejected");
}

void ClipboardCellsMimeTests::parseRejectsInvalidVersion() {
  const QByteArray invalid_payload =
      R"({"version":9999999,"rows":1,"cols":1,"cells":[{"content":"1","format":{"specifier":"###.##","rounding_mode":1,"word_wrap":false}}]})";

  QMimeData mime_data;
  mime_data.setData(KALQLATOR_CLIPBOARD_MIME_TYPE, invalid_payload);

  const auto parsed = parse_clipboard_cells_payload(&mime_data);
  QVERIFY2(!parsed.has_value(), "invalid version must be rejected");
}

void ClipboardCellsMimeTests::parseRejectsLowerVersion() {
  const QByteArray invalid_payload =
      R"({"version":0,"rows":1,"cols":1,"cells":[{"content":"1","format":{"specifier":"###.##","rounding_mode":1,"word_wrap":false}}]})";

  QMimeData mime_data;
  mime_data.setData(KALQLATOR_CLIPBOARD_MIME_TYPE, invalid_payload);

  const auto parsed = parse_clipboard_cells_payload(&mime_data);
  QVERIFY2(!parsed.has_value(), "invalid version must be rejected");
}

void ClipboardCellsMimeTests::parseRejectsMissingVersion() {
  const QByteArray no_version =
      R"({"rows":1,"cols":1,"cells":[{"content":"1"}]})";

  QMimeData mime_data;
  mime_data.setData(KALQLATOR_CLIPBOARD_MIME_TYPE, no_version);

  const auto parsed = parse_clipboard_cells_payload(&mime_data);
  QVERIFY2(!parsed.has_value(), "payload without version must be rejected");
}

QTEST_MAIN(ClipboardCellsMimeTests)
#include "test_clipboard_cells_mime.moc"
