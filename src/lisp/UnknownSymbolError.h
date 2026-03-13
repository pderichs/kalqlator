// KalQlator - UnknownSymbolError.h
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

#include <sstream>
#include <stdexcept>
#include <string>

namespace lisp {
    class UnknownSymbolError : public std::runtime_error {
    public:
        explicit UnknownSymbolError(const std::string& unknownSymbol)
            : std::runtime_error(buildMessage(unknownSymbol))
            , unknownSymbol_(unknownSymbol)
        {}

        [[nodiscard]] const std::string& symbol() const noexcept { return unknownSymbol_; }

    private:
        std::string unknownSymbol_;

        static std::string buildMessage(const std::string& symbol) {
            std::ostringstream oss;
            oss << "Unknown symbol: '" << symbol << "'";
            return oss.str();
        }
    };
}

