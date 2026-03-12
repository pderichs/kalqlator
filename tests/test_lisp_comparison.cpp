// KalQlator - test_lisp_comparison.cpp
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
#include <memory>
#include "TestBase.h"
#include "test_tools.h"
#include "../src/lisp/Evaluator.h"
#include "../src/tools/tools.h"
#include "../src/lisp/DefaultEnvironment.h"
#include "../src/lisp/tools.h"

using namespace lisp;

class LispComparisonTests : public TestBase {
    Q_OBJECT

private:
    EnvironmentPtr env;
    std::unique_ptr<Evaluator> evaluator;

    bool eval_bool(const std::string &expr) {
        auto lisp = parse_all_string(expr);
        return evaluator->evaluate(lisp)->is_true();
    }

private slots:
    void init() {
        env = std::make_shared<DefaultEnvironment>();
        evaluator = std::make_unique<Evaluator>(env, std::any{});
    }

    // >= Tests
    void test_gte_true_descending() {
        QVERIFY(eval_bool("(>= 10 8 7 3)"));
    }

    void test_gte_true_with_equal() {
        QVERIFY(eval_bool("(>= 10 10 5)"));
    }

    void test_gte_false() {
        QVERIFY(!eval_bool("(>= 5 10)"));
    }

    void test_gte_single_value() {
        QVERIFY(eval_bool("(>= 42)"));
    }

    // <= Tests
    void test_lte_true_ascending() {
        QVERIFY(eval_bool("(<= 1 2 5 10)"));
    }

    void test_lte_true_with_equal() {
        QVERIFY(eval_bool("(<= 5 5 10)"));
    }

    void test_lte_false() {
        QVERIFY(!eval_bool("(<= 10 5)"));
    }

    void test_lte_single_value() {
        QVERIFY(eval_bool("(<= 42)"));
    }

    // > Tests
    void test_gt_true_strict_descending() {
        QVERIFY(eval_bool("(> 10 5 2 1)"));
    }

    void test_gt_false_with_equal() {
        QVERIFY(!eval_bool("(> 10 10 5)"));
    }

    void test_gt_false() {
        QVERIFY(!eval_bool("(> 1 5)"));
    }

    void test_gt_single_value() {
        QVERIFY(eval_bool("(> 42)"));
    }

    // < Tests
    void test_lt_true_strict_ascending() {
        QVERIFY(eval_bool("(< 1 2 5 10)"));
    }

    void test_lt_false_with_equal() {
        QVERIFY(!eval_bool("(< 5 5 10)"));
    }

    void test_lt_false() {
        QVERIFY(!eval_bool("(< 10 5)"));
    }

    void test_lt_single_value() {
        QVERIFY(eval_bool("(< 42)"));
    }

    // Mixed type tests (int/double)
    void test_gte_mixed_types() {
        QVERIFY(eval_bool("(>= 10.0 8 7.5 3)"));
    }

    void test_lt_mixed_types() {
        QVERIFY(eval_bool("(< 1 2.5 3 4.0)"));
    }

    // Edge cases
    void test_comparison_two_values() {
        QVERIFY(eval_bool("(> 5 3)"));
        QVERIFY(eval_bool("(< 3 5)"));
        QVERIFY(eval_bool("(>= 5 5)"));
        QVERIFY(eval_bool("(<= 5 5)"));
    }

    void test_negative_numbers() {
        QVERIFY(eval_bool("(< -10 -5 0 5)"));
        QVERIFY(eval_bool("(> 5 0 -5 -10)"));
    }
};

QTEST_MAIN(LispComparisonTests)
#include "test_lisp_comparison.moc"