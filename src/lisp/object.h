// KalQlator - object.h
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


#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <variant>
#include "environment.h"
#include "types.h"

namespace lisp {
    struct Cons {
        LispObjectPtr car;
        LispObjectPtr cdr;
    };

    struct Symbol {
        std::string name;

        Symbol(std::string n) : name(std::move(n)) {
        }
    };

    using LispObjectPtrVector = std::vector<LispObjectPtr>;

    /**
     * Native defined functions inside the executable.
     *
     * These will be put into the default environment for each interpreter instance.
     */
    using NativeFn = std::function<LispObjectPtr(const LispObjectPtr &, const std::any&)>;

    struct NativeFuncDefinition {
        std::string name;
        NativeFn func;
    };

    /**
     * The user can define functions within the language.
     *
     * This struct combines the relevant name, params, body and environment.
     */
    struct UserFn {
        std::string name;
        LispObjectPtrVector params;
        LispObjectPtr body;
        EnvironmentPtr env;
    };

    struct Nil {
        constexpr bool operator==(const Nil &) const = default;
    };

    struct True {
        constexpr bool operator==(const True &) const = default;
    };

    struct LambdaFunction {
        LispObjectPtr args;
        LispObjectPtr body;
        EnvironmentPtr env;
    };

    using LispValue = std::variant<
        std::monostate,
        double,
        int64_t,
        std::string,
        True,
        Cons,
        Symbol,
        NativeFn,
        LambdaFunction,
        UserFn,
        Nil
    >;

    struct LispObject {
        LispValue data;

        [[nodiscard]] bool is_integer() const { return std::holds_alternative<Int64Type>(data); }
        [[nodiscard]] bool is_double() const { return std::holds_alternative<DoubleType>(data); }
        [[nodiscard]] bool is_string() const { return std::holds_alternative<std::string>(data); }
        [[nodiscard]] bool is_symbol() const { return std::holds_alternative<Symbol>(data); }
        [[nodiscard]] bool is_cons() const { return std::holds_alternative<Cons>(data); }
        [[nodiscard]] bool is_nil() const { return std::holds_alternative<Nil>(data); }
        [[nodiscard]] bool is_lambda() const { return std::holds_alternative<LambdaFunction>(data); }
        [[nodiscard]] bool is_number() const { return is_integer() || is_double(); }
        [[nodiscard]] bool is_atom() const { return !is_cons(); }

        [[nodiscard]] bool is_list() const {
            return is_nil() || (is_cons() && cdr()->is_list());
        }

        [[nodiscard]] LambdaFunction as_lambda() const {
            return std::get<LambdaFunction>(data);
        }

        [[nodiscard]] bool is_true() const { return std::holds_alternative<True>(data); }

        [[nodiscard]] bool is_empty_list() const {
            return is_nil(); // Nil is empty list
        }

        [[nodiscard]] LispObjectPtr car() const {
            const Cons &c = std::get<Cons>(data);
            return c.car;
        }

        [[nodiscard]] LispObjectPtr cdr() const {
            const Cons &c = std::get<Cons>(data);
            return c.cdr;
        }

        [[nodiscard]] Int64Type as_int64() const {
            return std::get<Int64Type>(data);
        }

        [[nodiscard]] std::string as_string() const {
            return std::get<std::string>(data);
        }

        [[nodiscard]] double as_double() const {
            return std::get<double>(data);
        }

        [[nodiscard]] Symbol as_symbol() const {
            return std::get<Symbol>(data);
        }

        [[nodiscard]] Cons as_cons() const {
            return std::get<Cons>(data);
        }

        [[nodiscard]] bool is_false() const {
            return is_nil();
        }

        [[nodiscard]] bool is_native_fn() const {
            return std::holds_alternative<NativeFn>(data);
        }

        [[nodiscard]] NativeFn as_native_fn() {
            return std::get<NativeFn>(data);
        }

        [[nodiscard]] std::string as_symbol_name() const {
            return as_symbol().name;
        }

        [[nodiscard]] bool is_truthy() const {
            return !is_nil();
        }
    };

    inline void debug_print(const LispObjectPtr &obj, std::ostream &out = std::cout) {
        if (!obj) {
            out << "<nullptr>";
            return;
        }

        std::visit([&out](auto &&arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, std::monostate>) {
                out << "<monostate>";
            } else if constexpr (std::is_same_v<T, double>) {
                out << arg;
            } else if constexpr (std::is_same_v<T, int64_t>) {
                out << arg;
            } else if constexpr (std::is_same_v<T, std::string>) {
                out << '"' << arg << '"';
            } else if constexpr (std::is_same_v<T, True>) {
                out << "T";
            } else if constexpr (std::is_same_v<T, Nil>) {
                out << "NIL";
            } else if constexpr (std::is_same_v<T, Symbol>) {
                out << arg.name;
            } else if constexpr (std::is_same_v<T, Cons>) {
                out << '(';
                debug_print(arg.car, out);

                LispObjectPtr current = arg.cdr;
                while (current && current->is_cons()) {
                    out << ' ';
                    debug_print(current->car(), out);
                    current = current->cdr();
                }

                if (current && !current->is_nil()) {
                    out << " . ";
                    debug_print(current, out);
                }
                out << ')';
            } else if constexpr (std::is_same_v<T, NativeFn>) {
                out << "<native-fn>";
            } else if constexpr (std::is_same_v<T, LambdaFunction>) {
                out << "<lambda ";
                debug_print(arg.args, out);
                debug_print(arg.body, out);
                out << '>';
            } else if constexpr (std::is_same_v<T, UserFn>) {
                out << "<fn " << arg.name << '>';
            }
        }, obj->data);

        out << std::endl;
    }

    // explicit overload for LispObject
    inline void debug_print(const LispObject &obj, std::ostream &out = std::cout) {
        auto ptr = std::make_shared<LispObject>(obj);
        debug_print(ptr, out);
    }
} // lisp

