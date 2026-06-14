// KalQlator - test_tools.cpp
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

#include "../src/tools/tools.h"
#include <QtTest/QtTest>

class ToolsTests : public QObject {
  Q_OBJECT

private slots:
  static void ltrimRemovesLeadingSpaces() {
    QCOMPARE(pdtools::ltrim("  hello"), "hello");
  }

  static void ltrimRemovesLeadingTabs() {
    QCOMPARE(pdtools::ltrim("\t\thello"), "hello");
  }

  static void ltrimDoesNothingForNoLeadingWhitespace() {
    QCOMPARE(pdtools::ltrim("hello"), "hello");
  }

  static void ltrimDoesNothingForEmptyString() {
    QCOMPARE(pdtools::ltrim(""), "");
  }

  static void rtrimRemovesTrailingSpaces() {
    QCOMPARE(pdtools::rtrim("hello  "), "hello");
  }

  static void rtrimRemovesTrailingTabs() {
    QCOMPARE(pdtools::rtrim("hello\t\t"), "hello");
  }

  static void rtrimDoesNothingForNoTrailingWhitespace() {
    QCOMPARE(pdtools::rtrim("hello"), "hello");
  }

  static void rtrimDoesNothingForEmptyString() {
    QCOMPARE(pdtools::rtrim(""), "");
  }

  static void trimRemovesBothSides() {
    QCOMPARE(pdtools::trim("  hello  "), "hello");
  }

  static void trimHandlesOnlyWhitespace() {
    QCOMPARE(pdtools::trim("   "), "");
  }

  static void trimDoesNothingForEmptyString() {
    QCOMPARE(pdtools::trim(""), "");
  }

  static void splitByComma() {
    auto result = pdtools::split("a,b,c", ',');
    QCOMPARE(result.size(), 3);
    QCOMPARE(result[0], "a");
    QCOMPARE(result[1], "b");
    QCOMPARE(result[2], "c");
  }

  static void splitBySemicolon() {
    auto result = pdtools::split("x;y;z", ';');
    QCOMPARE(result.size(), 3);
    QCOMPARE(result[0], "x");
    QCOMPARE(result[1], "y");
    QCOMPARE(result[2], "z");
  }

  static void splitWithEmptyParts() {
    auto result = pdtools::split("a,,c", ',');
    QCOMPARE(result.size(), 3);
    QCOMPARE(result[0], "a");
    QCOMPARE(result[1], "");
    QCOMPARE(result[2], "c");
  }

  static void splitSingleElement() {
    auto result = pdtools::split("hello", ',');
    QCOMPARE(result.size(), 1);
    QCOMPARE(result[0], "hello");
  }

  static void splitEmptyString() {
    auto result = pdtools::split("", ',');
    QCOMPARE(result.size(), 0);
  }

  static void splitTrimsParts() {
    auto result = pdtools::split(" a , b , c ", ',');
    QCOMPARE(result.size(), 3);
    QCOMPARE(result[0], "a");
    QCOMPARE(result[1], "b");
    QCOMPARE(result[2], "c");
  }

  static void stringVectorToStringJoinsWithCommas() {
    pdtools::StringVector vec = {"a", "b", "c"};
    QCOMPARE(pdtools::stringVectorToString(vec), "a,b,c,");
  }

  static void stringVectorToStringEmptyVector() {
    pdtools::StringVector vec;
    QCOMPARE(pdtools::stringVectorToString(vec), "");
  }

  static void intVectorToStringJoinsWithCommas() {
    pdtools::IntVector vec = {1, 2, 3};
    QCOMPARE(pdtools::intVectorToString(vec), "1,2,3,");
  }

  static void intVectorToStringEmptyVector() {
    pdtools::IntVector vec;
    QCOMPARE(pdtools::intVectorToString(vec), "");
  }

  static void locationToStringFormatsCorrectly() {
    Location loc(3, 7);
    QCOMPARE(pdtools::locationToString(loc), "(x: 3, y: 7)");
  }

  static void locationToStringZero() {
    Location loc(0, 0);
    QCOMPARE(pdtools::locationToString(loc), "(x: 0, y: 0)");
  }

  static void doubleNearlyEqReturnsTrueForEqualValues() {
    QVERIFY(pdtools::double_nearly_eq(1.0, 1.0, 0.0001));
  }

  static void doubleNearlyEqReturnsTrueWithinEpsilon() {
    QVERIFY(pdtools::double_nearly_eq(1.0, 1.00005, 0.0001));
  }

  static void doubleNearlyEqReturnsFalseOutsideEpsilon() {
    QVERIFY(!pdtools::double_nearly_eq(1.0, 1.001, 0.0001));
  }

  static void is_numberReturnsTrueForInteger() {
    QVERIFY(pdtools::is_number("42"));
  }

  static void is_numberReturnsTrueForFloat() {
    QVERIFY(pdtools::is_number("3.14"));
  }

  static void is_numberReturnsTrueForNegative() {
    QVERIFY(pdtools::is_number("-7"));
  }

  static void is_numberReturnsFalseForText() {
    QVERIFY(!pdtools::is_number("abc"));
  }

  static void is_numberReturnsFalseForEmptyString() {
    QVERIFY(!pdtools::is_number(""));
  }

  static void is_numberReturnsFalseForMixedContent() {
    QVERIFY(!pdtools::is_number("42abc"));
  }

  static void generate_uuid_returnsNonEmptyString() {
    std::string uuid = pdtools::generate_uuid();
    QVERIFY(!uuid.empty());
  }

  static void generate_uuid_hasCorrectFormat() {
    std::string uuid = pdtools::generate_uuid();
    // UUID format: 8-4-4-4-12 hex digits
    QCOMPARE(uuid.size(), 36);
    QCOMPARE(uuid[8], '-');
    QCOMPARE(uuid[13], '-');
    QCOMPARE(uuid[18], '-');
    QCOMPARE(uuid[23], '-');
  }

  static void generate_uuid_producesDifferentValues() {
    std::string uuid1 = pdtools::generate_uuid();
    std::string uuid2 = pdtools::generate_uuid();
    QVERIFY(uuid1 != uuid2);
  }

  static void convertStringToNumberInt() {
    auto result = pdtools::convert_string_to_number<int>("42");
    QVERIFY(result.has_value());
    QCOMPARE(result.value(), 42);
  }

  static void convertStringToNumberDouble() {
    auto result = pdtools::convert_string_to_number<double>("3.14");
    QVERIFY(result.has_value());
    QVERIFY(pdtools::double_nearly_eq(result.value(), 3.14, 0.0001));
  }

  static void convertStringToNumberReturnsNulloptForInvalid() {
    auto result = pdtools::convert_string_to_number<int>("abc");
    QVERIFY(!result.has_value());
  }

  static void convertStringToNumberReturnsNulloptForPartialMatch() {
    auto result = pdtools::convert_string_to_number<int>("42abc");
    QVERIFY(!result.has_value());
  }

  static void generateRandomIntInRangeWithinBounds() {
    for (int i = 0; i < 100; ++i) {
      int value = pdtools::generate_random_int_in_range(5, 10);
      QVERIFY(value >= 5);
      QVERIFY(value <= 10);
    }
  }

  static void generateRandomIntInRangeSingleValue() {
    for (int i = 0; i < 10; ++i) {
      int value = pdtools::generate_random_int_in_range(7, 7);
      QCOMPARE(value, 7);
    }
  }
};

QTEST_MAIN(ToolsTests)
#include "test_tools.moc"