// KalQlator - test_cell_display_formatter.cpp
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

#include "../src/lisp/factories.h"
#include "../src/model/CellDisplayFormatter.h"

class CellDisplayFormatterTests : public QObject {
  Q_OBJECT

private slots:
  static void parseNoPrefixNoPostfix();
  static void parseWithPostfix();
  static void parseWithPrefix();
  static void parseWithPrefixAndPostfix();
  static void invalidSpecifier();
  static void formatNearest();
  static void formatFloor();
  static void formatCeil();
  static void formatTruncate();
  static void formatNegativeNumbers();
  static void formatIntegers();
  static void formatNonNumericIsIgnored();
  static void noSpecifierReturnsOriginalValue();
  static void zeroDecimalPlaces();
  static void formatRationalArbitraryPrecision();
};

void CellDisplayFormatterTests::parseNoPrefixNoPostfix() {
  auto parsed = CellDisplayFormatter::parse_specifier("###.##");
  QVERIFY(parsed.valid);
  QCOMPARE(parsed.prefix, std::string());
  QCOMPARE(parsed.decimal_places, 2);
  QCOMPARE(parsed.postfix, std::string());
}

void CellDisplayFormatterTests::parseWithPostfix() {
  auto parsed = CellDisplayFormatter::parse_specifier("####.## EUR");
  QVERIFY(parsed.valid);
  QCOMPARE(parsed.prefix, std::string());
  QCOMPARE(parsed.decimal_places, 2);
  QCOMPARE(parsed.postfix, std::string(" EUR"));
}

void CellDisplayFormatterTests::parseWithPrefix() {
  auto parsed = CellDisplayFormatter::parse_specifier("EUR ####.##");
  QVERIFY(parsed.valid);
  QCOMPARE(parsed.prefix, std::string("EUR "));
  QCOMPARE(parsed.decimal_places, 2);
  QCOMPARE(parsed.postfix, std::string());
}

void CellDisplayFormatterTests::parseWithPrefixAndPostfix() {
  auto parsed = CellDisplayFormatter::parse_specifier("$ ###.## USD");
  QVERIFY(parsed.valid);
  QCOMPARE(parsed.prefix, std::string("$ "));
  QCOMPARE(parsed.decimal_places, 2);
  QCOMPARE(parsed.postfix, std::string(" USD"));
}

void CellDisplayFormatterTests::invalidSpecifier() {
  auto parsed = CellDisplayFormatter::parse_specifier("abc");
  QVERIFY(!parsed.valid);

  QVERIFY(!CellDisplayFormatter::is_valid_specifier("abc"));
  QVERIFY(!CellDisplayFormatter::is_valid_specifier(""));
  QVERIFY(!CellDisplayFormatter::is_valid_specifier("###.##.##"));
  QVERIFY(!CellDisplayFormatter::is_valid_specifier("EUR ### and ###"));
  QVERIFY(!CellDisplayFormatter::is_valid_specifier("##.#.#"));
  QVERIFY(CellDisplayFormatter::is_valid_specifier("###.##"));
}

void CellDisplayFormatterTests::formatNearest() {
  CellFormat fmt;
  fmt.specifier = "###.##";
  fmt.rounding_mode = RoundingMode::Nearest;

  QCOMPARE(CellDisplayFormatter::format("12.345", fmt), std::string("12.35"));
  QCOMPARE(CellDisplayFormatter::format("12", fmt), std::string("12.00"));
  QCOMPARE(CellDisplayFormatter::format("12.3", fmt), std::string("12.30"));
  QCOMPARE(CellDisplayFormatter::format("0", fmt), std::string("0.00"));
}

void CellDisplayFormatterTests::formatFloor() {
  CellFormat fmt;
  fmt.specifier = "###.##";
  fmt.rounding_mode = RoundingMode::Floor;

  QCOMPARE(CellDisplayFormatter::format("12.349", fmt), std::string("12.34"));
  QCOMPARE(CellDisplayFormatter::format("-12.349", fmt), std::string("-12.35"));
  QCOMPARE(CellDisplayFormatter::format("12.999", fmt), std::string("12.99"));
}

void CellDisplayFormatterTests::formatCeil() {
  CellFormat fmt;
  fmt.specifier = "###.##";
  fmt.rounding_mode = RoundingMode::Ceil;

  QCOMPARE(CellDisplayFormatter::format("12.341", fmt), std::string("12.35"));
  QCOMPARE(CellDisplayFormatter::format("-12.341", fmt), std::string("-12.34"));
  QCOMPARE(CellDisplayFormatter::format("12.001", fmt), std::string("12.01"));
}

void CellDisplayFormatterTests::formatTruncate() {
  CellFormat fmt;
  fmt.specifier = "###.##";
  fmt.rounding_mode = RoundingMode::Truncate;

  QCOMPARE(CellDisplayFormatter::format("12.349", fmt), std::string("12.34"));
  QCOMPARE(CellDisplayFormatter::format("-12.349", fmt), std::string("-12.34"));
  QCOMPARE(CellDisplayFormatter::format("12.999", fmt), std::string("12.99"));
}

void CellDisplayFormatterTests::formatNegativeNumbers() {
  CellFormat fmt;
  fmt.specifier = "###.##";
  fmt.rounding_mode = RoundingMode::Nearest;

  QCOMPARE(CellDisplayFormatter::format("-12.345", fmt), std::string("-12.35"));
  QCOMPARE(CellDisplayFormatter::format("-0.01", fmt), std::string("-0.01"));
}

void CellDisplayFormatterTests::formatIntegers() {
  CellFormat fmt;
  fmt.specifier = "##";
  fmt.rounding_mode = RoundingMode::Nearest;

  QCOMPARE(CellDisplayFormatter::format("42", fmt), std::string("42"));
  QCOMPARE(CellDisplayFormatter::format("0", fmt), std::string("0"));
  QCOMPARE(CellDisplayFormatter::format("-7", fmt), std::string("-7"));
}

void CellDisplayFormatterTests::formatNonNumericIsIgnored() {
  CellFormat fmt;
  fmt.specifier = "###.##";
  fmt.rounding_mode = RoundingMode::Nearest;

  QCOMPARE(CellDisplayFormatter::format("hello", fmt), std::string("hello"));
  QCOMPARE(CellDisplayFormatter::format("", fmt), std::string(""));
}

void CellDisplayFormatterTests::noSpecifierReturnsOriginalValue() {
  CellFormat fmt;
  QCOMPARE(CellDisplayFormatter::format("42", fmt), std::string("42"));
  QCOMPARE(CellDisplayFormatter::format("12.345", fmt), std::string("12.345"));
}

void CellDisplayFormatterTests::zeroDecimalPlaces() {
  CellFormat fmt;
  fmt.specifier = "##";
  fmt.rounding_mode = RoundingMode::Nearest;

  QCOMPARE(CellDisplayFormatter::format("12.7", fmt), std::string("13"));
  QCOMPARE(CellDisplayFormatter::format("12.4", fmt), std::string("12"));
}

void CellDisplayFormatterTests::formatRationalArbitraryPrecision() {
  CellFormat fmt;
  fmt.specifier = "###.##";
  fmt.rounding_mode = RoundingMode::Nearest;

  // Non-terminating decimal via string path
  QCOMPARE(CellDisplayFormatter::format("0.3333333333", fmt),
           std::string("0.33"));

  // Through format_number with a LispObject containing the same value
  auto obj = lisp::make_number("0.3333333333");
  QCOMPARE(CellDisplayFormatter::format_number(obj, fmt), std::string("0.33"));

  // 1/6 = 0.1666... rounds to 0.17
  auto obj2 = lisp::make_number("0.1666666667");
  QCOMPARE(CellDisplayFormatter::format_number(obj2, fmt), std::string("0.17"));

  // Single decimal place
  CellFormat fmt1;
  fmt1.specifier = "##.#";
  fmt1.rounding_mode = RoundingMode::Nearest;
  QCOMPARE(CellDisplayFormatter::format("0.3333333333", fmt1),
           std::string("0.3"));
}

QTEST_MAIN(CellDisplayFormatterTests)
#include "test_cell_display_formatter.moc"
