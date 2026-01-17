// KalQlator - test_lisp_quote.cpp
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

using namespace lisp;
class LispQuoteTests : public TestBase {
    Q_OBJECT
private slots:
    void quote_symbol();
    void quote_list();
};

void LispQuoteTests::quote_symbol() {
    // 5 and 5.0 should be identical
    LispObjectPtrVector lisp = parseAllString("(quote foo)");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    QVERIFY(result->is_symbol());
    QCOMPARE(result->as_symbol_name(), "foo");
}

void LispQuoteTests::quote_list() {
    // 5 and 5.0 should be identical
    LispObjectPtrVector lisp = parseAllString("(quote (1 \"Hello\" 3 5))");
    EnvironmentPtr env = std::make_shared<DefaultEnvironment>();
    Evaluator evaluator(env, {});
    auto result = evaluator.evaluate(lisp);
    checkCons(result, { ExpectedType::t_integer, 1, ExpectedType::t_string, "Hello", ExpectedType::t_integer, 3, ExpectedType::t_integer, 5 });
}
QTEST_MAIN(LispQuoteTests)
#include "test_lisp_quote.moc"