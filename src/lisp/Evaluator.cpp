// KalQlator - Evaluator.cpp
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

#include "Evaluator.h"

#include "ArgumentError.h"
#include "environment.h"
#include "EvalError.h"
#include "factories.h"
#include "SyntaxError.h"
#include "UnknownSymbolError.h"

namespace lisp {
    LispObjectPtr Evaluator::evaluate_list(const LispObjectPtr &list) {
        if (!list || list->is_nil()) {
            return make_nil();
        }

        const LispObjectPtr head = evaluate_object(list->car());
        const LispObjectPtr tail = evaluate_list(list->cdr());

        return make_cons(head, tail);
    }

    LispObjectPtr Evaluator::call_lambda(const LambdaFunction &lambda_function,
                                         const LispObjectPtr &args) {
        auto local_env = std::make_shared<Environment>(lambda_function.env);

        // Bind parameters in lambda local env
        auto param = lambda_function.args;
        auto arg = args;
        while (param && !param->is_nil()) {
            auto param_name = param->car()->as_symbol_name();
            auto arg_value = evaluate_object(arg->car());
            local_env->define(param_name, arg_value);

            param = param->cdr();
            arg = arg->cdr();
        }

        // Evaluate body - last result is returned
        auto saved_env = env_;
        env_ = local_env;

        LispObjectPtr result = make_nil();
        auto expr = lambda_function.body;
        while (expr && !expr->is_nil()) {
            result = evaluate_object(expr->car());
            expr = expr->cdr();
        }

        env_ = saved_env;
        return result;
    }

    LispObjectPtr Evaluator::eval_function(const LispObjectPtr &list) {
        auto function = list->car();

        // We could encounter a Cons here - this would mean we could have a lamba
        // as a function.
        if (function->is_cons()) {
            // Evaluate the first part again to support a possible lambda inline
            // definition and call.
            function = evaluate_object(function);
        }

        if (function->is_lambda()) {
            // "inline" lambda
            return call_lambda(function->as_lambda(), list->cdr());
        }

        if (!function->is_symbol()) {
            throw SyntaxError("Function must be either a symbol or a lambda.");
        }

        // Native function
        const std::string function_name = function->as_symbol_name();

        const auto fn_definition = env_->lookup(function_name);

        if (fn_definition) {
            if (fn_definition->is_native_fn()) {
                const auto function_impl = fn_definition->as_native_fn();
                const auto args = list->cdr();
                env_->on_pre_function_eval_args(function_name, args, context_param_);
                const LispObjectPtr evaluated_args = evaluate_list(args); // eval all arguments
                return function_impl(evaluated_args, context_param_);
            }

            if (fn_definition->is_lambda()) {
                // Lambda defined in env
                return call_lambda(fn_definition->as_lambda(), list->cdr());
            }
        }

        throw UnknownSymbolError(function_name);
    }

    bool Evaluator::is_named_function(const LispObjectPtr &obj, const std::string &function_name) {
        return obj->is_symbol() && obj->as_symbol_name() == function_name;
    }

    LispObjectPtr Evaluator::evaluate_object(const LispObjectPtr &obj) {
        if (!obj) {
            return make_nil();
        }

        if (obj->is_nil()) {
            return make_nil();
        }

        if (obj->is_symbol()) {
            return env_->lookup(obj->as_symbol_name());
        }

        if (obj->is_atom()) {
            return obj;
        }

        if (obj->is_cons()) {
            // Special form "quote"
            if (is_named_function(obj->car(), "quote")) {
                return obj->cdr()->car();
            }

            // Special forms "setq" or "define"
            bool is_setq = is_named_function(obj->car(), "setq");
            bool is_define = is_named_function(obj->car(), "define");
            if (is_setq || is_define) {
                auto symbol_name = obj->cdr()->car()->as_symbol_name();
                auto symbol_def = obj->cdr()->cdr()->car();
                evaluate_object(symbol_def);

                // debug_print(symbol_def);
                if (is_setq) {
                    env_->set(symbol_name, evaluate_object(symbol_def));
                } else {
                    env_->define(symbol_name, evaluate_object(symbol_def));
                }

                return symbol_def;
            }

            // Special form "if"
            if (is_named_function(obj->car(), "if")) {
                auto condition = evaluate_object(obj->cdr()->car());
                if (obj->cdr()->cdr()->is_nil()) {
                    throw ArgumentError("if without result");
                }

                auto condition_fulfilled = obj->cdr()->cdr()->car();
                LispObjectPtr condition_not_fulfilled;

                auto false_condition = obj->cdr()->cdr()->cdr();
                if (false_condition->is_nil()) {
                    condition_not_fulfilled = make_nil();
                } else {
                    condition_not_fulfilled = false_condition->car();
                }

                if (condition->is_truthy()) {
                    return evaluate_object(condition_fulfilled);
                }

                return evaluate_object(condition_not_fulfilled);
            }

            // Special form "lambda" - no evaluation should take place.
            if (is_named_function(obj->car(), "lambda")) {
                auto lambda_args = obj->cdr()->car();
                auto lambda_body = obj->cdr()->cdr();
                auto lambda = make_lambda(lambda_args, lambda_body, env_);

                return lambda;
            }

            return eval_function(obj);
        }

        throw EvalError("Unable to evaluate.");
    }

    LispObjectPtr Evaluator::evaluate(const LispObjectPtrVector &objects) {
        LispObjectPtr last;
        for (const auto &lisp_object: objects) {
            last = evaluate_object(lisp_object);
        }

        if (last) {
            return last;
        }

        return make_nil();
    }
} // lisp
