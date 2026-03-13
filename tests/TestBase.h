// KalQlator - TestBase.h
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

#pragma once

#include <QObject>

#include "../src/lisp/types.h"
#include "../src/lisp/object.h"
#include "../src/lisp/tokenizer/lisp_tokenizer.h"
#include "../src/lisp/parser/parser.h"

namespace lisp {
    class TestBase: public QObject {
    protected:
        static LispObjectPtr parseString(const std::string &input) {
            Tokenizer tokenizer(input);
            LispTokens tokens = tokenizer.scan();
            Parser parser(tokens);
            return parser.parse();
        }

        template<typename ExceptionType>
        static void expectParseError(const std::string &input) {
            bool exceptionThrown = false;
            try {
                parseString(input);
            } catch (const ExceptionType &) {
                exceptionThrown = true;
            } catch (...) {
                QFAIL("Wrong exception type thrown");
            }
            QVERIFY2(exceptionThrown,
                     ("Expected exception for input: " + input).c_str());
        }
    };
} // lisp

