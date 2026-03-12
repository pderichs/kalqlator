// KalQlator - TableCell.h
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

#include "../../lisp/object.h"
#include "../../lisp/tools.h"
#include "../../lisp/ArgumentError.h"
#include "../../lisp/factories.h"
#include "../SheetRegistry.h"

const lisp::NativeFn FnTableCell = lisp::NativeFn{
    [](const lisp::LispObjectPtr &args, const std::any &context_param) -> lisp::LispObjectPtr {
        if (countListElements(args) != 2) {
            throw lisp::ArgumentError("TableCell requires two arguments");
        }

        const auto &table = args->car();
        const auto &cell_ref = args->cdr()->car();

        if (!table->is_string()) {
            throw lisp::ArgumentError("Table reference must be of type string.");
        }

        if (!cell_ref->is_string()) {
            throw lisp::ArgumentError("Cell reference must be of type string.");
        }

        const std::string table_ref = table->as_string();
        // Future: we could also parse here for a prefix like "id:" to support also id references.

        const auto &context = std::any_cast<TableContext>(context_param);

        const auto sheet = context.sheet_registry->sheet_by_name(table_ref);
        const auto &cell_symbol = cell_ref->as_string();
        const auto &env = sheet->environment();
        if (!env->is_defined(cell_symbol)) {
            throw lisp::ArgumentError("Cell value must be defined.");
        }

        return env->lookup(cell_symbol);
    }
};
