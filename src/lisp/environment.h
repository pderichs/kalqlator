// KalQlator - environment.h
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
#include <map>
#include <memory>

#include "types.h"

namespace lisp {
using DefinitionMap = std::map<std::string, LispObjectPtr>;

class Environment;
using EnvironmentPtr = std::shared_ptr<Environment>;

class Environment {
public:
  explicit Environment(EnvironmentPtr parent);
  virtual ~Environment() = default;

  /**
   * Creates a definition in the current environment only.
   *
   * @param name Name of the value
   * @param value Value to be set
   */
  virtual void define(const std::string &name, LispObjectPtr value);

  /**
   * Mutates a possible existing definition. ("set!")
   *
   * @param name Name of the value
   * @param value Value to be set
   */
  virtual void set(const std::string &name, const LispObjectPtr &value);

  [[nodiscard]] bool is_defined(const std::string &name) const;

  /**
   * Looks a symbol up.
   *
   * @param name Name to lookup
   * @return Value of the found symbol or Nil
   */
  LispObjectPtr lookup(const std::string &name);

  /**
   * This is called before function arguments are evaluated.
   */
  virtual void on_pre_function_eval_args(const std::string & /*unused*/,
                                         const LispObjectPtr & /*unused*/,
                                         const std::any & /*unused*/) {}

private:
  /**
   * Mutates an existing definition up in the hierarchy.
   * If an update happened, function returns true, false otherwise.
   *
   * @param name Name of the value
   * @param value Value to be set
   * @return true if value was set, false otherwise.
   */
  bool try_update(const std::string &name, const LispObjectPtr &value);

  /**
   * Environment chain
   */
  EnvironmentPtr parent_;

  /**
   * Definitions for this environment
   */
  DefinitionMap definitions_;
};

} // namespace lisp
