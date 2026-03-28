// KalQlator - factories.h
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

#include <format>
#include <utility>

#include "object.h"

namespace lisp {
inline LispObjectPtr make_number(NumberRepresentation number_representation) {
  auto result = std::make_shared<LispObject>();
  result->data = std::move(number_representation);
  return result;
}

template <typename T> LispObjectPtr make_number(T value) {
  const auto string = std::format("{}", value);
  return make_number(NumberRepresentation{string});
}

template <typename T> LispObjectPtr make_singleton() {
  static LispObjectPtr instance = []() {
    auto object = std::make_shared<LispObject>();
    object->data = T{};
    return object;
  }();
  return instance;
}

inline LispObjectPtr make_nil() { return make_singleton<Nil>(); }

inline LispObjectPtr make_true() { return make_singleton<True>(); }

inline LispObjectPtr make_bool(const bool value) {
  if (value) {
    return make_true();
  }

  return make_nil();
}

inline LispObjectPtr make_string(std::string value) {
  auto result = std::make_shared<LispObject>();
  result->data = std::move(value);
  return result;
}

inline LispObjectPtr make_symbol(std::string value) {
  auto result = std::make_shared<LispObject>();
  result->data = Symbol(std::move(value));
  return result;
}

inline LispObjectPtr make_cons(LispObjectPtr car, LispObjectPtr cdr) {
  auto result = std::make_shared<LispObject>();
  result->data = Cons(std::move(car), std::move(cdr));
  return result;
}

inline LispObjectPtr make_lambda(LispObjectPtr args, LispObjectPtr body,
                                 EnvironmentPtr env) {
  auto result = std::make_shared<LispObject>();
  result->data = LambdaFunction{
      .args = std::move(args), .body = std::move(body), .env = std::move(env)};
  return result;
}

inline LispObjectPtr make_native_fn(NativeFn function) {
  auto result = std::make_shared<LispObject>();
  result->data = std::move(function);
  return result;
}

} // namespace lisp
