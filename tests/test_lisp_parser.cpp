// KalQlator - test_lisp_parser.cpp
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

#include "TestBase.h"
#include "test_tools.h"
#include "../src/lisp/tokenizer/lisp_tokenizer.h"
#include "../src/lisp/parser/parser.h"
#include "../src/lisp/tokenizer/lisp_parser_error.h"
#include "../src/tools/tools.h"
#include "../src/lisp/tools.h"

using namespace lisp;

class LispParserTests : public TestBase {
    Q_OBJECT

private slots:
    // === atoms ===
    static void atom_integer();

    static void atom_negative_integer();
    static void atom_double();
    static void atom_negative_double();
    static void atom_string();
    static void atom_string_with_spaces();
    static void atom_symbol();
    static void atom_nil();
    static void atom_true();
    static void empty_list_is_nil1();
    static void empty_list_is_nil2();

    // === simple lists ===
    static void simple_list();

    static void empty_list();
    static void single_element_list();

    // === nested lists ===
    static void nested_list();
    static void deeply_nested_list();

    // === quoted expressions ===
    static void quoted_symbol();
    static void quoted_list();

    // === edge cases ===
    static void whitespace_handling();
    static void multiple_spaces();

    // === complex expressions ===
    static void arithmetic_expression();
    static void function_call_with_nested_args();
    static void list_with_mixed_types();

    // === error handling ===
    static void error_unmatched_open_paren();
    static void error_unmatched_close_paren();
    static void error_extra_close_paren();
    static void error_unterminated_string();
    static void error_empty_input();
    static void error_only_whitespace();

    // === edge cases: numbers ===
    static void number_zero();
    static void number_negative_zero();
    static void number_leading_plus();
    static void number_very_large_integer();
    static void number_very_small_double();
    static void number_double_without_leading_digit();  // ".5"
    static void symbol_with_two_dots_at_the_start();
    static void number_double_without_trailing_digit(); // "5."

    // === string edge cases ===
    static void string_empty();
    static void string_with_escaped_quote();
    static void string_with_newline();
    static void string_with_special_chars();

    // === symbol edge cases ===
    static void symbol_with_numbers();
    static void symbol_starting_with_number_like();
    static void symbol_special_chars();
    static void symbol_plus_minus_star_slash();
    static void symbol_question_mark();
    static void symbol_exclamation_mark();
    static void symbol_comparison_operators();
    static void symbol_uppercase();
    static void symbol_mixed_case();

    // === dotted pairs ===
    static void dotted_pair_simple();
    static void dotted_pair_with_lists();
    static void improper_list();
    static void error_dot_at_start();
    static void error_dot_without_cdr();
    static void error_multiple_dots();

    // === multiple expressions ===
    static void multiple_top_level_atoms();
    static void multiple_top_level_atoms_parse_all();

    static void multiple_top_level_lists();

    static void multiple_top_level_lists_parse_all();

    static void parse_all_expressions();

    static void parse_all_expressions_parse_all();

    // === quoting variants ===
    static void nested_quotes();
    static void quoted_empty_list();
    static void quoted_nil();
    static void quote_at_end_of_list();

    // === comments ===
    static void comment_line();
    static void comment_at_end_of_line();
    static void comment_between_expressions();
    static void comment_inside_list();

    // === complex structures ===
    static void lambda_expression();
    static void let_binding();
    static void cond_expression();
    static void deeply_nested_arithmetic();
    static void list_of_lists();

    // === boundary conditions ===
    static void very_long_list();
    static void very_deep_nesting();
    static void very_long_symbol();
    static void very_long_string();
};

void LispParserTests::atom_integer() {
    LispObjectPtr result = parseString("42");
    QVERIFY(result);
    QVERIFY(result->is_integer());
    QCOMPARE(result->as_int64(), 42);
}

void LispParserTests::atom_negative_integer() {
    LispObjectPtr result = parseString("-123");
    QVERIFY(result);
    QVERIFY(result->is_integer());
    QCOMPARE(result->as_int64(), -123);
}

void LispParserTests::atom_double() {
    LispObjectPtr result = parseString("3.14159");
    QVERIFY(result);
    QVERIFY(result->is_double());
    QCOMPARE(result->as_double(), 3.14159);
}

void LispParserTests::atom_negative_double() {
    LispObjectPtr result = parseString("-2.5");
    QVERIFY(result);
    QVERIFY(result->is_double());
    QCOMPARE(result->as_double(), -2.5);
}

void LispParserTests::atom_string() {
    LispObjectPtr result = parseString("\"hello\"");
    QVERIFY(result);
    QVERIFY(result->is_string());
    QCOMPARE(result->as_string(), "hello");
}

void LispParserTests::atom_string_with_spaces() {
    LispObjectPtr result = parseString("\"hello world\"");
    QVERIFY(result);
    QVERIFY(result->is_string());
    QCOMPARE(result->as_string(), "hello world");
}

void LispParserTests::atom_symbol() {
    LispObjectPtr result = parseString("foo-bar");
    QVERIFY(result);
    QVERIFY(result->is_symbol());
    QCOMPARE(result->as_symbol_name(), "foo-bar");
}

void LispParserTests::atom_nil() {
    LispObjectPtr result = parseString("nil");
    QVERIFY(result);
    QVERIFY(result->is_nil());
}

void LispParserTests::atom_true() {
    LispObjectPtr result = parseString("t");
    QVERIFY(result);
    QVERIFY(result->is_true());
}

void LispParserTests::empty_list_is_nil1() {
    LispObjectPtr result = parseString("()");
    QVERIFY(result);
    QVERIFY(result->is_nil());
}

void LispParserTests::empty_list_is_nil2() {
    LispObjectPtr result = parseString("   (     )   ");
    QVERIFY(result);
    QVERIFY(result->is_nil());
}

void LispParserTests::simple_list() {
    LispObjectPtr result = parseString("(+ 22 4)");
    QVERIFY(result);
    QVERIFY(result->is_cons());
    QVERIFY(result->car()->is_symbol());
    QCOMPARE(result->car()->as_symbol_name(), "+");

    LispObjectPtr rest = result->cdr();
    QVERIFY(rest->is_cons());
    QVERIFY(rest->car()->is_integer());
    QCOMPARE(rest->car()->as_int64(), 22);

    rest = rest->cdr();
    QVERIFY(rest->is_cons());
    QVERIFY(rest->car()->is_integer());
    QCOMPARE(rest->car()->as_int64(), 4);
}

void LispParserTests::empty_list() {
    LispObjectPtr result = parseString("()");
    QVERIFY(result);
    QVERIFY(result->is_nil());
}

void LispParserTests::single_element_list() {
    LispObjectPtr result = parseString("(42)");
    QVERIFY(result);
    QVERIFY(result->is_cons());
    QVERIFY(result->car()->is_integer());
    QCOMPARE(result->car()->as_int64(), 42);
    QVERIFY(result->cdr()->is_nil());
}

void LispParserTests::nested_list() {
    LispObjectPtr result = parseString("(+ 1 (* 2 3))");
    QVERIFY(result);
    QVERIFY(result->is_cons());

    QVERIFY(result->car()->is_symbol());
    QCOMPARE(result->car()->as_symbol_name(), "+");

    LispObjectPtr rest = result->cdr();
    QVERIFY(rest->is_cons());
    QVERIFY(rest->car()->is_integer());
    QCOMPARE(rest->car()->as_int64(), 1);

    rest = rest->cdr();
    QVERIFY(rest->is_cons());
    LispObjectPtr inner = rest->car();
    checkCons(inner, { ExpectedType::t_symbol, "*", ExpectedType::t_integer, 2, ExpectedType::t_integer, 3 });
}

void LispParserTests::deeply_nested_list() {
    LispObjectPtr result = parseString("((()))");
    QVERIFY(result);
    QVERIFY(result->is_cons());
    QVERIFY(result->car()->is_cons());
    QVERIFY(result->car()->car()->is_nil());
}

void LispParserTests::quoted_symbol() {
    LispObjectPtr result = parseString("'foo");
    QVERIFY(result);
    QVERIFY(result->is_cons());
    QVERIFY(result->car()->is_symbol());
    QCOMPARE(result->car()->as_symbol_name(), "quote");
    QVERIFY(result->cdr()->is_cons());
    QVERIFY(result->cdr()->car()->is_symbol());
    QCOMPARE(result->cdr()->car()->as_symbol_name(), "foo");
}

void LispParserTests::quoted_list() {
    LispObjectPtr result = parseString("'(1 2 3)");
    QVERIFY(result);
    QVERIFY(result->is_cons());
    QVERIFY(result->car()->is_symbol());
    QCOMPARE(result->car()->as_symbol_name(), "quote");

    LispObjectPtr quoted = result->cdr()->car();
    checkCons(quoted, { ExpectedType::t_integer, 1, ExpectedType::t_integer, 2, ExpectedType::t_integer, 3 });
}

void LispParserTests::whitespace_handling() {
    LispObjectPtr result = parseString("  (  +   1   2  )  ");
    QVERIFY(result);
    QVERIFY(result->is_cons());
    checkCons(result, { ExpectedType::t_symbol, "+", ExpectedType::t_integer, 1, ExpectedType::t_integer, 2});
}

void LispParserTests::multiple_spaces() {
    LispObjectPtr result = parseString("(a    b    c)");
    QVERIFY(result);
    checkCons(result, { ExpectedType::t_symbol, "a", ExpectedType::t_symbol, "b", ExpectedType::t_symbol, "c" });
}

void LispParserTests::arithmetic_expression() {
    LispObjectPtr result = parseString("(/ (+ 10 20) (- 5 2))");
    QVERIFY(result);
    QVERIFY(result->is_cons());
    QVERIFY(result->car()->is_symbol());
    QCOMPARE(result->car()->as_symbol_name(), "/");

    LispObjectPtr first_arg = result->cdr()->car();
    checkCons(first_arg, { ExpectedType::t_symbol, "+", ExpectedType::t_integer, 10, ExpectedType::t_integer, 20 });

    LispObjectPtr second_arg = result->cdr()->cdr()->car();
    checkCons(second_arg, { ExpectedType::t_symbol, "-", ExpectedType::t_integer, 5, ExpectedType::t_integer, 2 });
}

void LispParserTests::function_call_with_nested_args() {
    LispObjectPtr result = parseString("(defun square (x) (* x x))");
    QVERIFY(result);
    QVERIFY(result->is_cons());
    QVERIFY(result->car()->is_symbol());
    QCOMPARE(result->car()->as_symbol_name(), "defun");

    LispObjectPtr fname = result->cdr()->car();
    QVERIFY(fname->is_symbol());
    QCOMPARE(fname->as_symbol_name(), "square");

    LispObjectPtr params = result->cdr()->cdr()->car();
    QVERIFY(params->is_cons());
    QVERIFY(params->car()->is_symbol());
    QCOMPARE(params->car()->as_symbol_name(), "x");
    QVERIFY(params->cdr()->is_nil());

    LispObjectPtr body = result->cdr()->cdr()->cdr()->car();
    checkCons(body, { ExpectedType::t_symbol, "*", ExpectedType::t_symbol, "x", ExpectedType::t_symbol, "x"});
}

void LispParserTests::list_with_mixed_types() {
    LispObjectPtr result = parseString("(1 \"hello\" 3.14 foo nil)");
    QVERIFY(result);
    QVERIFY(result->is_cons());

    QVERIFY(result->car()->is_integer());
    QCOMPARE(result->car()->as_int64(), 1);

    result = result->cdr();
    QVERIFY(result->car()->is_string());
    QCOMPARE(result->car()->as_string(), "hello");

    result = result->cdr();
    QVERIFY(result->car()->is_double());
    QCOMPARE(result->car()->as_double(), 3.14);

    result = result->cdr();
    QVERIFY(result->car()->is_symbol());
    QCOMPARE(result->car()->as_symbol_name(), "foo");

    result = result->cdr();
    QVERIFY(result->car()->is_nil());
    QVERIFY(result->cdr()->is_nil());
}

void LispParserTests::error_unmatched_open_paren() {
    expectParseError<LispParserError>("(");
    expectParseError<LispParserError>("(+ 1");
    expectParseError<LispParserError>("((()");
}

void LispParserTests::error_unmatched_close_paren() {
    expectParseError<LispParserError>(")");
    expectParseError<LispParserError>("1)");
}

void LispParserTests::error_extra_close_paren() {
    expectParseError<LispParserError>("(+ 1 2))");
    expectParseError<LispParserError>("())");
}

void LispParserTests::error_unterminated_string() {
    expectParseError<LispParserError>("\"hello");
    expectParseError<LispParserError>("\"");
    expectParseError<LispParserError>("(print \"unterminated)");
}

void LispParserTests::error_empty_input() {
    LispObjectPtr result = parseString("");
    QVERIFY(result->is_nil());
}

void LispParserTests::error_only_whitespace() {
    LispObjectPtr result = parseString("   \t\n   ");
    QVERIFY(result->is_nil());
}

void LispParserTests::number_zero() {
    LispObjectPtr result = parseString("0");
    QVERIFY(result);
    QVERIFY(result->is_integer());
    QCOMPARE(result->as_int64(), 0);
}

void LispParserTests::number_negative_zero() {
    LispObjectPtr result = parseString("-0");
    QVERIFY(result);
    QVERIFY(result->is_integer());
    QCOMPARE(result->as_int64(), 0);
}

void LispParserTests::number_leading_plus() {
    LispObjectPtr result = parseString("+42");
    QVERIFY(result);
    QVERIFY(result->is_integer());
    QCOMPARE(result->as_int64(), 42);
}

void LispParserTests::number_very_large_integer() {
    LispObjectPtr result = parseString("9223372036854775807");  // INT64_MAX
    QVERIFY(result);
    QVERIFY(result->is_integer());
    QCOMPARE(result->as_int64(), INT64_MAX);
}

void LispParserTests::number_very_small_double() {
    LispObjectPtr result = parseString("0.0000001");
    QVERIFY(result);
    QVERIFY(result->is_double());
    QVERIFY(qFuzzyCompare(result->as_double(), 0.0000001));
}

void LispParserTests::number_double_without_leading_digit() {
    LispObjectPtr result = parseString(".5");
    QVERIFY(result);
    QVERIFY(result->is_double());
    QCOMPARE(result->as_double(), 0.5);
}

void LispParserTests::symbol_with_two_dots_at_the_start() {
    LispObjectPtr result = parseString("..5");
    QVERIFY(result);
    QVERIFY(result->is_symbol());
    QCOMPARE(result->as_symbol_name(), "..5");
}

void LispParserTests::number_double_without_trailing_digit() {
    LispObjectPtr result = parseString("5.");
    QVERIFY(result);
    QVERIFY(result->is_double());
    QCOMPARE(result->as_double(), 5.0);
}

void LispParserTests::string_empty() {
    LispObjectPtr result = parseString("\"\"");
    QVERIFY(result);
    QVERIFY(result->is_string());
    QCOMPARE(result->as_string(), "");
}

void LispParserTests::string_with_escaped_quote() {
    LispObjectPtr result = parseString(R"("hello\"world")");
    QVERIFY(result);
    QVERIFY(result->is_string());
    QCOMPARE(result->as_string(), "hello\"world");
}

void LispParserTests::string_with_newline() {
    LispObjectPtr result = parseString(R"("line1\nline2")");
    QVERIFY(result);
    QVERIFY(result->is_string());
    QCOMPARE(result->as_string(), "line1\nline2");
}

void LispParserTests::string_with_special_chars() {
    LispObjectPtr result = parseString(R"("tab:\there\\backslash")");
    QVERIFY(result);
    QVERIFY(result->is_string());
    QCOMPARE(result->as_string(), "tab:\there\\backslash");
}

void LispParserTests::symbol_with_numbers() {
    LispObjectPtr result = parseString("foo123");
    QVERIFY(result);
    QVERIFY(result->is_symbol());
    QCOMPARE(result->as_symbol_name(), "foo123");
}

void LispParserTests::symbol_starting_with_number_like() {
    LispObjectPtr result = parseString("1+");
    QVERIFY(result);
    QVERIFY(result->is_symbol());
    QCOMPARE(result->as_symbol_name(), "1+");
}

void LispParserTests::symbol_special_chars() {
    LispObjectPtr result = parseString("foo-bar_baz");
    QVERIFY(result);
    QVERIFY(result->is_symbol());
    QCOMPARE(result->as_symbol_name(), "foo-bar_baz");
}

void LispParserTests::symbol_plus_minus_star_slash() {
    LispObjectPtr plus = parseString("+");
    QVERIFY(plus && plus->is_symbol());
    QCOMPARE(plus->as_symbol_name(), "+");

    LispObjectPtr minus = parseString("-");
    QVERIFY(minus && minus->is_symbol());
    QCOMPARE(minus->as_symbol_name(), "-");

    LispObjectPtr star = parseString("*");
    QVERIFY(star && star->is_symbol());
    QCOMPARE(star->as_symbol_name(), "*");

    LispObjectPtr slash = parseString("/");
    QVERIFY(slash && slash->is_symbol());
    QCOMPARE(slash->as_symbol_name(), "/");
}

void LispParserTests::symbol_question_mark() {
    LispObjectPtr result = parseString("null?");
    QVERIFY(result);
    QVERIFY(result->is_symbol());
    QCOMPARE(result->as_symbol_name(), "null?");
}

void LispParserTests::symbol_exclamation_mark() {
    LispObjectPtr result = parseString("set!");
    QVERIFY(result);
    QVERIFY(result->is_symbol());
    QCOMPARE(result->as_symbol_name(), "set!");
}

void LispParserTests::symbol_comparison_operators() {
    LispObjectPtr lower_than = parseString("<");
    QVERIFY(lower_than && lower_than->is_symbol());
    QCOMPARE(lower_than->as_symbol_name(), "<");

    LispObjectPtr lower_than_equal = parseString("<=");
    QVERIFY(lower_than_equal && lower_than_equal->is_symbol());
    QCOMPARE(lower_than_equal->as_symbol_name(), "<=");

    LispObjectPtr greater_than = parseString(">");
    QVERIFY(greater_than && greater_than->is_symbol());
    QCOMPARE(greater_than->as_symbol_name(), ">");

    LispObjectPtr greater_than_equal = parseString(">=");
    QVERIFY(greater_than_equal && greater_than_equal->is_symbol());
    QCOMPARE(greater_than_equal->as_symbol_name(), ">=");

    LispObjectPtr equal = parseString("=");
    QVERIFY(equal && equal->is_symbol());
    QCOMPARE(equal->as_symbol_name(), "=");
}

void LispParserTests::symbol_uppercase() {
    LispObjectPtr result = parseString("HELLO");
    QVERIFY(result);
    QVERIFY(result->is_symbol());
    QCOMPARE(result->as_symbol_name(), "HELLO");
}

void LispParserTests::symbol_mixed_case() {
    LispObjectPtr result = parseString("HelloWorld");
    QVERIFY(result);
    QVERIFY(result->is_symbol());
    QCOMPARE(result->as_symbol_name(), "HelloWorld");
}

void LispParserTests::dotted_pair_simple() {
    LispObjectPtr result = parseString("(a . b)");
    QVERIFY(result);
    QVERIFY(result->is_cons());
    QVERIFY(result->car()->is_symbol());
    QCOMPARE(result->car()->as_symbol_name(), "a");
    QVERIFY(result->cdr()->is_symbol());
    QCOMPARE(result->cdr()->as_symbol_name(), "b");
}

void LispParserTests::dotted_pair_with_lists() {
    LispObjectPtr result = parseString("((1 2) . (3 4))");
    QVERIFY(result);
    QVERIFY(result->is_cons());

    QVERIFY(result->car()->is_cons());  // (1 2)
    auto firstCons = result->car();
    QVERIFY(firstCons->car()->is_integer());
    QCOMPARE(firstCons->car()->as_int64(), 1);
    auto rest = firstCons->cdr();
    QVERIFY(rest->is_cons());
    QCOMPARE(rest->car()->as_int64(), 2);
    QVERIFY(rest->cdr()->is_nil()); // Last element

    QVERIFY(result->cdr()->is_cons());  // (3 4)
    auto secondCons = result->cdr();
    QVERIFY(secondCons->car()->is_integer());
    QCOMPARE(secondCons->car()->as_int64(), 3);
    rest = secondCons->cdr();
    QVERIFY(rest->is_cons());
    QCOMPARE(rest->car()->as_int64(), 4);
    QVERIFY(rest->cdr()->is_nil()); // Last element
}

void LispParserTests::improper_list() {
    LispObjectPtr result = parseString("(1 2 . 3)");
    QVERIFY(result);
    QVERIFY(result->is_cons());
    QCOMPARE(result->car()->as_int64(), 1);

    LispObjectPtr rest = result->cdr();
    QVERIFY(rest->is_cons());
    QCOMPARE(rest->car()->as_int64(), 2);

    QVERIFY(rest->cdr()->is_integer());
    QCOMPARE(rest->cdr()->as_int64(), 3);
}

void LispParserTests::error_dot_at_start() {
    expectParseError<LispParserError>("(. a b)");
}

void LispParserTests::error_dot_without_cdr() {
    expectParseError<LispParserError>("(a .)");
}

void LispParserTests::error_multiple_dots() {
    expectParseError<LispParserError>("(a . b . c)");
}

void LispParserTests::multiple_top_level_atoms() {
    LispObjectPtr result = parseString("1 2 3");
    QVERIFY(result);
    QVERIFY(result->is_integer());
    QCOMPARE(result->as_int64(), 1);
}

void LispParserTests::multiple_top_level_atoms_parse_all() {
    LispObjectPtrVector result = parse_all_string("1 2 3");
    QVERIFY(result.size() == 3);

    auto item = result.front();
    QVERIFY(item->is_integer());
    QCOMPARE(item->as_int64(), 1);

    item = result[1];
    QVERIFY(item->is_integer());
    QCOMPARE(item->as_int64(), 2);

    item = result[2];
    QVERIFY(item->is_integer());
    QCOMPARE(item->as_int64(), 3);
}

void LispParserTests::multiple_top_level_lists() {
    LispObjectPtr result = parseString("(+ 1 2) (- 3 4)");
    QVERIFY(result);
    QVERIFY(result->is_cons());
    checkCons(result, { ExpectedType::t_symbol, "+", ExpectedType::t_integer, 1, ExpectedType::t_integer, 2 });
}

void LispParserTests::multiple_top_level_lists_parse_all() {
    LispObjectPtrVector result = parse_all_string("(+ 1 2) (- 3 4)");
    QVERIFY(result.size() == 2);

    auto item = result.front();
    checkCons(item, { ExpectedType::t_symbol, "+", ExpectedType::t_integer, 1, ExpectedType::t_integer, 2 });

    item = result[1];
    checkCons(item, { ExpectedType::t_symbol, "-", ExpectedType::t_integer, 3, ExpectedType::t_integer, 4 });
}

void LispParserTests::parse_all_expressions() {
    Tokenizer tokenizer("1 2 3");
    LispTokens tokens = tokenizer.scan();
    Parser parser(tokens);

    LispObjectPtr first = parser.parse();
    QVERIFY(first && first->is_integer());
    QCOMPARE(first->as_int64(), 1);
}

void LispParserTests::parse_all_expressions_parse_all() {
    Tokenizer tokenizer("1 2 3");
    LispTokens tokens = tokenizer.scan();
    Parser parser(tokens);

    LispObjectPtrVector all = parser.parse_all();

    auto item = all.front();
    QVERIFY(item && item->is_integer());
    QCOMPARE(item->as_int64(), 1);

    item = all[1];
    QVERIFY(item && item->is_integer());
    QCOMPARE(item->as_int64(), 2);

    item = all[2];
    QVERIFY(item && item->is_integer());
    QCOMPARE(item->as_int64(), 3);
}

void LispParserTests::nested_quotes() {
    // ''foo -> (quote (quote foo))
    LispObjectPtr result = parseString("''foo");
    QVERIFY(result);
    QVERIFY(result->is_cons());
    QCOMPARE(result->car()->as_symbol_name(), "quote");

    LispObjectPtr inner = result->cdr()->car();
    QVERIFY(inner->is_cons());
    QCOMPARE(inner->car()->as_symbol_name(), "quote");
    QCOMPARE(inner->cdr()->car()->as_symbol_name(), "foo");
}

void LispParserTests::quoted_empty_list() {
    // '() -> (quote nil)
    LispObjectPtr result = parseString("'()");
    QVERIFY(result);
    QVERIFY(result->is_cons());
    QCOMPARE(result->car()->as_symbol_name(), "quote");
    QVERIFY(result->cdr()->car()->is_nil());
}

void LispParserTests::quoted_nil() {
    // 'nil -> (quote nil)
    LispObjectPtr result = parseString("'nil");
    QVERIFY(result);
    QVERIFY(result->is_cons());
    QCOMPARE(result->car()->as_symbol_name(), "quote");
    QVERIFY(result->cdr()->car()->is_nil());
}

void LispParserTests::quote_at_end_of_list() {
    // (foo 'bar) -> (foo (quote bar))
    LispObjectPtr result = parseString("(foo 'bar)");
    QVERIFY(result);
    QVERIFY(result->is_cons());
    QCOMPARE(result->car()->as_symbol_name(), "foo");

    LispObjectPtr quoted = result->cdr()->car();
    QVERIFY(quoted->is_cons());
    QCOMPARE(quoted->car()->as_symbol_name(), "quote");
    QCOMPARE(quoted->cdr()->car()->as_symbol_name(), "bar");
}

void LispParserTests::comment_line() {
    LispObjectPtr result = parseString("; this is a comment\n42");
    QVERIFY(result);
    QVERIFY(result->is_integer());
    QCOMPARE(result->as_int64(), 42);
}

void LispParserTests::comment_at_end_of_line() {
    LispObjectPtr result = parseString("42 ; trailing comment");
    QVERIFY(result);
    QVERIFY(result->is_integer());
    QCOMPARE(result->as_int64(), 42);
}

void LispParserTests::comment_between_expressions() {
    LispObjectPtr result = parseString("(+ 1 ; add one\n 2)");
    QVERIFY(result);
    QVERIFY(result->is_cons());
    QCOMPARE(countListElements(result), 3);

    QCOMPARE(result->car()->as_symbol_name(), "+");
    auto cdr = result->cdr();
    QCOMPARE(cdr->car()->as_int64(), 1);
    cdr = cdr->cdr();
    QCOMPARE(cdr->car()->as_int64(), 2);
}

void LispParserTests::comment_inside_list() {
    LispObjectPtr result = parseString("(a ; comment\n b c)");
    QVERIFY(result);
    QVERIFY(result->is_cons());
    QCOMPARE(countListElements(result), 3);

    QCOMPARE(result->car()->as_symbol_name(), "a");
    auto cdr = result->cdr();
    QCOMPARE(cdr->car()->as_symbol_name(), "b");
    cdr = cdr->cdr();
    QCOMPARE(cdr->car()->as_symbol_name(), "c");
}

void LispParserTests::lambda_expression() {
    LispObjectPtr result = parseString("(lambda (x y) (+ x y))");
    QVERIFY(result);
    QVERIFY(result->is_cons());
    QCOMPARE(result->car()->as_symbol_name(), "lambda");

    // params
    LispObjectPtr params = result->cdr()->car();
    checkCons(params, { ExpectedType::t_symbol, "x", ExpectedType::t_symbol, "y" });

    // body
    LispObjectPtr body = result->cdr()->cdr()->car();
    checkCons(body, { ExpectedType::t_symbol, "+", ExpectedType::t_symbol, "x", ExpectedType::t_symbol, "y" });
}

void LispParserTests::let_binding() {
    LispObjectPtr result = parseString("(let ((x 1) (y 2)) (+ x y))");
    QVERIFY(result);
    QVERIFY(result->is_cons());
    QCOMPARE(result->car()->as_symbol_name(), "let");

    // Bindings
    LispObjectPtr bindings = result->cdr()->car();
    QVERIFY(bindings->is_cons());

    // First binding (x 1)
    LispObjectPtr first_binding = bindings->car();
    checkCons(first_binding, { ExpectedType::t_symbol, "x", ExpectedType::t_integer, 1 });

    // Second binding (y 2)
    LispObjectPtr second_binding = bindings->cdr()->car();
    checkCons(second_binding, { ExpectedType::t_symbol, "y", ExpectedType::t_integer, 2 });

    // Body
    LispObjectPtr body = result->cdr()->cdr()->car();
    checkCons(body, { ExpectedType::t_symbol, "+", ExpectedType::t_symbol, "x", ExpectedType::t_symbol, "y" });
}

void LispParserTests::cond_expression() {
    LispObjectPtr result = parseString(
        R"((cond ((< x 0) "negative") ((> x 0) "positive") (t "zero")))");
    QVERIFY(result);
    QVERIFY(result->is_cons());
    QCOMPARE(result->car()->as_symbol_name(), "cond");

    // Three clauses
    LispObjectPtr clauses = result->cdr();
    QCOMPARE(countListElements(clauses), 3);

    LispObjectPtr first = clauses->car();
    QVERIFY(first->is_cons());
    auto first_condition = first->car();
    checkCons(first_condition, { ExpectedType::t_symbol, "<", ExpectedType::t_symbol, "x", ExpectedType::t_integer, 0 });
    auto first_value = first->cdr()->car();
    QVERIFY(first_value->is_string());
    QCOMPARE(first_value->as_string(), "negative");

    LispObjectPtr second = clauses->cdr()->car();
    QVERIFY(second->is_cons());
    auto second_condition = second->car();
    checkCons(second_condition, { ExpectedType::t_symbol, ">", ExpectedType::t_symbol, "x", ExpectedType::t_integer, 0 });
    auto second_value = second->cdr()->car();
    QVERIFY(second_value->is_string());
    QCOMPARE(second_value->as_string(), "positive");

    LispObjectPtr third = clauses->cdr()->cdr()->car();
    QVERIFY(third->is_cons());
    auto third_condition = third->car();
    QVERIFY(third_condition->is_true());
    auto third_value = third->cdr()->car();
    QVERIFY(third_value->is_string());
    QCOMPARE(third_value->as_string(), "zero");
}

void LispParserTests::deeply_nested_arithmetic() {
    LispObjectPtr result = parseString("(+ (* 2 (- 10 5)) (/ 100 (+ 3 2)))");
    QVERIFY(result);
    QVERIFY(result->is_cons());
    QCOMPARE(result->car()->as_symbol_name(), "+");

    // First operand: (* 2 (- 10 5))
    LispObjectPtr first = result->cdr()->car();
    QVERIFY(first->is_cons());
    QCOMPARE(first->car()->as_symbol_name(), "*");
    QCOMPARE(first->cdr()->car()->as_int64(), 2);
    auto second_product = first->cdr()->cdr()->car();
    checkCons(second_product, { ExpectedType::t_symbol, "-", ExpectedType::t_integer, 10, ExpectedType::t_integer, 5 });

    // Second operand: (/ 100 (+ 3 2))
    LispObjectPtr second = result->cdr()->cdr()->car();
    QVERIFY(second->is_cons());
    QCOMPARE(second->car()->as_symbol_name(), "/");
    QCOMPARE(second->cdr()->car()->as_int64(), 100);
    auto second_part = second->cdr()->cdr()->car();
    checkCons(second_part, { ExpectedType::t_symbol, "+", ExpectedType::t_integer, 3, ExpectedType::t_integer, 2 });
}

void LispParserTests::list_of_lists() {
    LispObjectPtr result = parseString("((1 2) (3 4) (5 6))");
    QVERIFY(result);
    QVERIFY(result->is_cons());
    QCOMPARE(countListElements(result), 3);

    // Every element is a list itself with two elements
    LispObjectPtr current = result;
    while (current->is_cons()) {
        QVERIFY(current->car()->is_cons());
        QCOMPARE(countListElements(current->car()), 2);
        current = current->cdr();
    }
}

void LispParserTests::very_long_list() {
    // Create list with 1000 elements
    std::string input = "(";
    for (int i = 0; i < 1000; ++i) {
        input += std::to_string(i) + " ";
    }
    input += ")";

    LispObjectPtr result = parseString(input);
    QVERIFY(result);
    QVERIFY(result->is_cons());
    QCOMPARE(countListElements(result), 1000);
}

void LispParserTests::very_deep_nesting() {
    // 100 levels deep
    std::string input;
    for (int i = 0; i < 100; ++i) {
        input += "(";
    }
    input += "42";
    for (int i = 0; i < 100; ++i) {
        input += ")";
    }

    LispObjectPtr result = parseString(input);
    QVERIFY(result);

    // Navigate to inner element
    LispObjectPtr current = result;
    for (int i = 0; i < 100; ++i) {
        QVERIFY(current->is_cons());
        current = current->car();
    }
    QVERIFY(current->is_integer());
    QCOMPARE(current->as_int64(), 42);
}

void LispParserTests::very_long_symbol() {
    std::string long_symbol(1000, 'a');
    LispObjectPtr result = parseString(long_symbol);
    QVERIFY(result);
    QVERIFY(result->is_symbol());
    QCOMPARE(result->as_symbol_name().length(), 1000UL);
    QCOMPARE(result->as_symbol_name(), std::string(1000, 'a'));
}

void LispParserTests::very_long_string() {
    std::string long_string = "\"" + std::string(10000, 'x') + "\"";
    LispObjectPtr result = parseString(long_string);
    QVERIFY(result);
    QVERIFY(result->is_string());
    QCOMPARE(result->as_string().length(), 10000UL);
    QCOMPARE(result->as_string(), std::string(10000, 'x'));
}

QTEST_MAIN(LispParserTests)
#include "test_lisp_parser.moc"