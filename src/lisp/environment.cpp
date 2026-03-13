// KalQlator - environment.cpp
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

#include "environment.h"

#include <utility>
#include "factories.h"
#include "UnknownSymbolError.h"

using namespace lisp;

Environment::Environment(EnvironmentPtr parent): parent_(std::move(parent)) {
}

void Environment::define(const std::string &name, LispObjectPtr value) {
    definitions_[name] = std::move(value);
}

void Environment::set(const std::string &name, const LispObjectPtr &value) {
    if (try_update(name, value)) {
        return;
    }

    define(name, value);
}

bool Environment::is_defined(const std::string &name) const {
    return definitions_.contains(name);
}

LispObjectPtr Environment::lookup(const std::string &name) {
    auto iterator = definitions_.find(name);
    if (iterator != definitions_.end()) {
        return iterator->second;
    }

    if (parent_) {
        return parent_->lookup(name);
    }

    throw UnknownSymbolError(name);
}

bool Environment::try_update(const std::string &name, const LispObjectPtr &value) {
    if (is_defined(name)) {
        // Update
        define(name, value);
        return true;
    }

    if (parent_) {
        return parent_->try_update(name, value);
    }

    return false;
}
