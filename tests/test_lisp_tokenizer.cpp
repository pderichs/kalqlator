// KalQlator - test_lisp_tokenizer.cpp
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

#include <any>
#include <QtTest/QtTest>
#include "../src/lisp/tokenizer/lisp_tokenizer.h"
#include "../src/tools/tools.h"

using namespace lisp;

class LispTokenizerTests : public QObject {
    Q_OBJECT

private slots:
    static void tokenizerTest1();

    static void tokenizerTest2();

    static void tokenizerTest3();

    static void tokenizerTest4();

    static void atomTest1();

    static void atomTest2();

    static void atomTest3();

    static void symbolTest1();

    static void quoteTest1();

    static void quoteTest2();

    static void numberWithNewLine();
};

void LispTokenizerTests::tokenizerTest1() {
    Tokenizer parser("(+ 22 4)");
    LispTokens tokens = parser.scan();

    QCOMPARE(tokens.size(), 7U);
    QCOMPARE(tokens[0].id, OPEN_BRACKET);
    QCOMPARE(tokens[1].id, IDENTIFIER);
    QCOMPARE(tokens[2].id, SPACE);
    QCOMPARE(tokens[3].id, INTEGER);
    QCOMPARE(tokens[4].id, SPACE);
    QCOMPARE(tokens[5].id, INTEGER);
    QCOMPARE(tokens[6].id, CLOSE_BRACKET);
}

void LispTokenizerTests::tokenizerTest2() {
    Tokenizer parser(R"lisp((some-function "Hello \"World\"" 22 4))lisp");
    LispTokens tokens = parser.scan();

    QCOMPARE(tokens.size(), 9U);
    QCOMPARE(tokens[0].id, OPEN_BRACKET);
    QCOMPARE(tokens[1].id, IDENTIFIER);

    auto actual_content = std::any_cast<std::string>(tokens[1].content);
    QCOMPARE(actual_content, std::string("some-function"));

    QCOMPARE(tokens[2].id, SPACE);
    QCOMPARE(tokens[3].id, STRING);

    actual_content = std::any_cast<std::string>(tokens[3].content);
    QCOMPARE(actual_content, std::string("Hello \"World\""));

    QCOMPARE(tokens[4].id, SPACE);
    QCOMPARE(tokens[5].id, INTEGER);

    auto actual_value = std::any_cast<Int64Type>(tokens[5].content);
    QCOMPARE(actual_value, 22.0);

    QCOMPARE(tokens[6].id, SPACE);
    QCOMPARE(tokens[7].id, INTEGER);

    actual_value = std::any_cast<Int64Type>(tokens[7].content);
    QCOMPARE(actual_value, 4.0);

    QCOMPARE(tokens[8].id, CLOSE_BRACKET);
}

void LispTokenizerTests::tokenizerTest3() {
    Tokenizer parser("(hello \"8282\" -484.32)");
    LispTokens tokens = parser.scan();

    QCOMPARE(tokens.size(), 7U);
    QCOMPARE(tokens[0].id, OPEN_BRACKET);
    QCOMPARE(tokens[1].id, IDENTIFIER);

    auto actual_content = std::any_cast<std::string>(tokens[1].content);
    QCOMPARE(actual_content, std::string("hello"));

    QCOMPARE(tokens[2].id, SPACE);
    QCOMPARE(tokens[3].id, STRING);

    actual_content = std::any_cast<std::string>(tokens[3].content);
    QCOMPARE(actual_content, std::string("8282"));

    QCOMPARE(tokens[4].id, SPACE);
    QCOMPARE(tokens[5].id, DOUBLE);

    auto actual_value = std::any_cast<DoubleType>(tokens[5].content);
    QCOMPARE(actual_value, -484.32);

    QCOMPARE(tokens[6].id, CLOSE_BRACKET);
}

void LispTokenizerTests::tokenizerTest4() {
    Tokenizer parser("(funktion1 \"(* 5.344 22)\" \")\")");
    LispTokens tokens = parser.scan();

    QCOMPARE(tokens.size(), 7U);
    QCOMPARE(tokens[0].id, OPEN_BRACKET);
    QCOMPARE(tokens[1].id, IDENTIFIER);

    auto actual_content = std::any_cast<std::string>(tokens[1].content);
    QCOMPARE(actual_content, std::string("funktion1"));

    QCOMPARE(tokens[2].id, SPACE);
    QCOMPARE(tokens[3].id, STRING);

    actual_content = std::any_cast<std::string>(tokens[3].content);
    QCOMPARE(actual_content, std::string("(* 5.344 22)"));

    QCOMPARE(tokens[4].id, SPACE);
    QCOMPARE(tokens[5].id, STRING);

    actual_content = std::any_cast<std::string>(tokens[5].content);
    QCOMPARE(actual_content, std::string(")"));

    QCOMPARE(tokens[6].id, CLOSE_BRACKET);
}

void LispTokenizerTests::atomTest1() {
    Tokenizer parser("42");
    LispTokens tokens = parser.scan();

    QCOMPARE(tokens.size(), 1U);
    QCOMPARE(tokens[0].id, INTEGER);
    QCOMPARE(std::any_cast<Int64Type>(tokens[0].content), 42);
}

void LispTokenizerTests::atomTest2() {
    Tokenizer parser("\"hello\"");
    LispTokens tokens = parser.scan();

    QCOMPARE(tokens.size(), 1U);
    QCOMPARE(tokens[0].id, STRING);
    QCOMPARE(std::any_cast<std::string>(tokens[0].content), "hello");
}

void LispTokenizerTests::atomTest3() {
    Tokenizer parser("32.5684");
    LispTokens tokens = parser.scan();

    QCOMPARE(tokens.size(), 1U);
    QCOMPARE(tokens[0].id, DOUBLE);
    QCOMPARE(std::any_cast<DoubleType>(tokens[0].content), 32.5684);
}


void LispTokenizerTests::symbolTest1() {
    Tokenizer parser("(some-function )");
    LispTokens tokens = parser.scan();

    QCOMPARE(tokens.size(), 4U);
    QCOMPARE(tokens[0].id, OPEN_BRACKET);
    QCOMPARE(tokens[1].id, IDENTIFIER);

    auto actual_value = std::any_cast<std::string>(tokens[1].content);
    QCOMPARE(actual_value, std::string("some-function"));

    QCOMPARE(tokens[2].id, SPACE);

    QCOMPARE(tokens[3].id, CLOSE_BRACKET);
}

void LispTokenizerTests::quoteTest1() {
    Tokenizer tokenizer("'foo");
    LispTokens tokens = tokenizer.scan();

    QCOMPARE(tokens.size(), 2U);
    QCOMPARE(tokens[0].id, IDENTIFIER);
    QCOMPARE(tokens[1].id, IDENTIFIER);

    auto actual_content = std::any_cast<std::string>(tokens[0].content);
    QCOMPARE(actual_content, std::string("'"));

    actual_content = std::any_cast<std::string>(tokens[1].content);
    QCOMPARE(actual_content, std::string("foo"));
}

void LispTokenizerTests::quoteTest2() {
    Tokenizer tokenizer("''foo");
    LispTokens tokens = tokenizer.scan();

    QCOMPARE(tokens.size(), 3U);
    QCOMPARE(tokens[0].id, IDENTIFIER);
    QCOMPARE(tokens[1].id, IDENTIFIER);
    QCOMPARE(tokens[2].id, IDENTIFIER);

    auto actual_content = std::any_cast<std::string>(tokens[0].content);
    QCOMPARE(actual_content, std::string("'"));

    actual_content = std::any_cast<std::string>(tokens[1].content);
    QCOMPARE(actual_content, std::string("'"));

    actual_content = std::any_cast<std::string>(tokens[2].content);
    QCOMPARE(actual_content, std::string("foo"));
}

void LispTokenizerTests::numberWithNewLine() {
    Tokenizer tokenizer("1\n");
    LispTokens tokens = tokenizer.scan();

    QCOMPARE(tokens.size(), 2U);
    QCOMPARE(tokens[0].id, INTEGER);
    QCOMPARE(std::any_cast<Int64Type>(tokens[0].content), 1);

    QCOMPARE(tokens[1].id, SPACE);
}

QTEST_MAIN(LispTokenizerTests)
#include "test_lisp_tokenizer.moc"