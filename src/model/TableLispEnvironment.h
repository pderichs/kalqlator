// KalQlator - TableLispEnvironment.h
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

#include "../lisp/DefaultEnvironment.h"
#include "../tools/tools.h"

using NameReferencesMap = std::map<std::string, pdtools::StringSet>;

/**
 * This stores the cell values with their respective lisp values.
 */
class TableLispEnvironment : public lisp::DefaultEnvironment {
public:
    void initialize();

    TableLispEnvironment() = default;

    void define(const std::string &name, lisp::LispObjectPtr value) override;

    void remove_references(const std::string &name);

    void set(const std::string &name, const lisp::LispObjectPtr &value) override;


    void update_references(const std::string &from_cell, const std::string &to_cell);

    /**
     * This is called before function arguments are evaluated.
     *
     * @param function_name Function name
     * @param args Function arguments about to be evaluated
     * @param context_param Context specific parameter from Evaluator
     */
    void on_pre_function_eval_args(const std::string &function_name, const lisp::LispObjectPtr &args,
                                   const std::any &context_param) override;

private:
    void dfs(const std::string &cell, pdtools::StringSet &visited, pdtools::StringVector &result);

    pdtools::StringVector dependency_chain_in_topological_order(const std::string &cell);

    bool is_reachable(const std::string &start, const std::string &target);

    void signal_environment_update(const std::string &name, lisp::LispObjectPtr value);

    /**
     * This map stores all cells which are referenced by the key cell
     *
     * example: Cell A1 has the formula "=(cell A2)"
     * So the entry here is:
     * "A1": { "A2" }
     */
    NameReferencesMap references_;

    /**
     * This map stores all cells which reference the key cell
     *
     * example: Cell A1 has the formula "=(cell A2)"
     * So the entry here is:
     * "A2": { "A1" }
     */
    NameReferencesMap referenced_by_;

    bool initializing_{false};
};

using TableLispEnvironmentPtr = std::shared_ptr<TableLispEnvironment>;
