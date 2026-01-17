// KalQlator - Evaluator.h
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

#include <any>
#include <utility>

#include "environment.h"
#include "object.h"
#include "types.h"

namespace lisp {
    /**
     * Interprets Lisp values with a defined environment.
     */
    class Evaluator {
    public:
        explicit Evaluator(EnvironmentPtr env, std::any context_param) : env_(std::move(env)),
                                                                       context_param_(std::move(context_param)) {
        }

        LispObjectPtr evaluate_object(const LispObjectPtr &shared);

        LispObjectPtr evaluate(const LispObjectPtrVector &objects);

    private:
        LispObjectPtr evaluate_list(const LispObjectPtr &list);

        LispObjectPtr call_lambda(const LambdaFunction &lambda_function, const LispObjectPtr &cdr);

        LispObjectPtr eval_function(const LispObjectPtr &list);

        static bool is_named_function(const LispObjectPtr &obj, const std::string &function_name);

        EnvironmentPtr env_;

        std::any context_param_;
    };
} // lisp
