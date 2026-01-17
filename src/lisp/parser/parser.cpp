// KalQlator - parser.cpp
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

#include "parser.h"

#include <ranges>
#include <stack>

#include "../factories.h"
#include "../tokenizer/lisp_parser_error.h"
#include "../tokenizer/lisp_syntax_checker.h"
#include "../tokenizer/lisp_tokenizer.h"

using namespace lisp;

LispObjectPtr Parser::parse() const {
    return parse_all().front();
}

LispObjectPtrVector Parser::parse_all() const {
    if (count_relevant_tokens() == 0) {
        return {make_nil()};
    }

    LispSyntaxChecker checker(_tokens);
    checker.check();

    std::stack<std::vector<LispObjectPtr> > stack;
    stack.emplace(); // Empty vector

    size_t ignore_token_count = 0;
    size_t current_token_pos = 0;

    for (auto const &token: _tokens) {
        if (ignore_token_count > 0) {
            ignore_token_count--;
            current_token_pos++;
            continue;
        }

        switch (token.id) {
            case OPEN_BRACKET: {
                stack.emplace(); // Empty vector
                break;
            }

            case CLOSE_BRACKET: {
                auto elements = std::move(stack.top());
                stack.pop();

                if (elements.empty()) {
                    // Empty list is nil
                    stack.top().push_back(make_nil());
                } else {
                    auto list = vector_to_cons(elements);
                    stack.top().push_back(list);
                }
                break;
            }

            case STRING: {
                stack.top().push_back(make_string(std::any_cast<std::string>(token.content)));
                break;
            }

            case INTEGER: {
                stack.top().push_back(make_int(std::any_cast<Int64Type>(token.content)));
                break;
            }

            case IDENTIFIER: {
                auto symbolName = std::any_cast<std::string>(token.content);
                if (symbolName == "nil") {
                    stack.top().push_back(make_nil());
                } else if (symbolName == "t") {
                    stack.top().push_back(make_true());
                } else if (symbolName == "'") {
                    // Special case quoting: We recursively read the expression to be quoted and
                    // put it into a quoted list. Advancement is done via ignore_token_count.
                    LispTokens expression = read_expression(current_token_pos + 1, &ignore_token_count);
                    Parser parser(expression);
                    LispObjectPtr quoted_expression = parser.parse();
                    LispObjectPtr cons = make_cons(
                        make_symbol("quote"),
                        make_cons(quoted_expression, make_nil()));
                    stack.top().push_back(cons);
                } else {
                    stack.top().push_back(make_symbol(symbolName));
                }
                break;
            }

            case DOUBLE: {
                stack.top().push_back(make_double(std::any_cast<DoubleType>(token.content)));
                break;
            }

            default:
                // Just continue.
                break;
        }

        current_token_pos++;
    }

    auto top = stack.top();
    return top;
}

std::optional<size_t> Parser::find_dot_position(const LispObjectPtrVector &elements) {
    size_t pos = 0;
    for (auto const &element: elements) {
        if (element->is_symbol() && element->as_symbol_name() == ".") {
            return pos;
        }

        pos++;
    }

    return {};
}

LispObjectPtr Parser::build_dotted_list(const LispObjectPtrVector &vector, unsigned long dot_position) {
    LispObjectPtrVector left(vector.begin(), vector.begin() + dot_position);

    if (left.empty()) {
        throw LispParserError("Nothing before '.'");
    }

    LispObjectPtrVector right(vector.begin() + dot_position + 1, vector.end());
    if (right.size() > 1) {
        throw LispParserError("Right part of . must be 1 element.");
    }

    LispObjectPtr tail = right.back();

    LispObjectPtr result = tail;
    for (auto &it: std::ranges::reverse_view(left)) {
        result = make_cons(it, result);
    }

    return result;
}

LispObjectPtr Parser::vector_to_cons(const LispObjectPtrVector &elements) {
    if (elements.empty()) {
        return make_nil();
    }

    LispObjectPtr result = make_nil();

    // If there is a dot we need to provide a cons cell with the two accompanying elements.
    // First check if we have this situation:
    auto dot_position = find_dot_position(elements);
    if (dot_position == std::nullopt) {
        for (const auto &element: std::ranges::reverse_view(elements)) {
            result = make_cons(element, result);
        }
    } else {
        if (*dot_position != elements.size() - 2) {
            throw LispParserError("Malformed dotted pair");
        }

        result = build_dotted_list(elements, *dot_position);
    }

    return result;
}

LispTokens Parser::read_expression(size_t current_token_pos, size_t *ignore_token_count) const {
    LispTokens result;

    size_t i = current_token_pos;
    bool exit = false;
    size_t in_list = 0;

    while (!exit && i < _tokens.size()) {
        const LispToken token = _tokens.at(i);

        switch (token.id) {
            case SPACE:
                if (!result.empty() && in_list <= 0) {
                    exit = true;
                } else {
                    // Ignore
                    i++;
                    continue;
                }
                break;
            case OPEN_BRACKET:
                in_list++;
                result.push_back(token);
                break;
            case IDENTIFIER:
            case INTEGER:
            case DOUBLE:
            case STRING:
                result.push_back(token);
                break;
            case CLOSE_BRACKET:
                if (in_list > 0) {
                    // If there were opened brackets within this run
                    // we need to add this token.
                    result.push_back(token);
                    in_list--;
                } else {
                    // If there were no open brackets within
                    // this scan, we can interpret this as the
                    // end of the expression.
                    exit = true;

                    // Jump to condition of while directly without
                    // increasing "i" here (closing bracket should
                    // not be ignored!)
                    continue;
                }
                break;
        }

        i++;
    }

    *ignore_token_count = i - current_token_pos;

    return result;
}

size_t Parser::count_relevant_tokens() const {
    size_t sum = 0;
    for (auto const &token: _tokens) {
        if (token.id != SPACE) {
            sum++;
        }
    }
    return sum;
}
