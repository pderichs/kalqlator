#include <QtTest>

#include "../src/lisp/LispObjectStringConverter.h"
#include "../src/lisp/factories.h"
#include "TestBase.h"

using namespace lisp;

class TestLispObjectStringConverter : public lisp::TestBase {
  Q_OBJECT

private slots:
  // --- Numbers ---
  static void test_positive_integer() {
    auto obj = make_number({"42"});
    LispObjectStringConverter conv(obj);
    QCOMPARE(conv.to_str(), std::string("42"));
  }

  static void test_negative_integer() {
    // Regression: "(- 1 44)" must not result in "-4.3"
    auto obj = make_number({"-43"});
    LispObjectStringConverter conv(obj);
    QCOMPARE(conv.to_str(), std::string("-43"));
  }

  static void test_zero() {
    auto obj = make_number({"0"});
    LispObjectStringConverter conv(obj);
    QCOMPARE(conv.to_str(), std::string("0"));
  }

  static void test_positive_fraction() {
    // 1/2 = 0.5
    auto obj = make_number({"1/2"});
    LispObjectStringConverter conv(obj);
    QCOMPARE(conv.to_str(), std::string("0.5"));
  }

  static void test_negative_fraction() {
    // -1/4 = -0.25
    auto obj = make_number({"-1/4"});
    LispObjectStringConverter conv(obj);
    QCOMPARE(conv.to_str(), std::string("-0.25"));
  }

  static void test_large_integer() {
    auto obj = make_number({"1000000"});
    LispObjectStringConverter conv(obj);
    QCOMPARE(conv.to_str(), std::string("1000000"));
  }

  static void test_negative_large_integer() {
    auto obj = make_number({"-1000000"});
    LispObjectStringConverter conv(obj);
    QCOMPARE(conv.to_str(), std::string("-1000000"));
  }

  // --- Strings ---
  static void test_string_value() {
    auto obj = make_string("hello");
    LispObjectStringConverter conv(obj);
    QCOMPARE(conv.to_str(), std::string("hello"));
  }

  static void test_empty_string() {
    auto obj = make_string("");
    LispObjectStringConverter conv(obj);
    QCOMPARE(conv.to_str(), std::string(""));
  }

  // --- Symbols ---
  static void test_symbol() {
    auto obj = make_symbol("my-var");
    LispObjectStringConverter conv(obj);
    QCOMPARE(conv.to_str(), std::string("my-var"));
  }

  // --- Fallback ---
  static void test_unknown_type_returns_placeholder() {
    auto obj = std::make_shared<LispObject>(); // Empty object
    LispObjectStringConverter conv(obj);
    QCOMPARE(conv.to_str(), std::string("<value>"));
  }
};

QTEST_MAIN(TestLispObjectStringConverter)
#include "test_lisp_object_string_converter.moc"