cmake_minimum_required(VERSION 3.31.6)

find_package(Qt6 REQUIRED COMPONENTS Test)

set(ADDITIONAL_DEPS
        ../src/lisp/object.h
        ../src/lisp/parser/parser.h
        ../src/lisp/parser/parser.cpp
        ../src/lisp/environment.h
        ../src/lisp/native/calculation.h
        ../src/lisp/factories.h
        ../src/lisp/tokenizer/lisp_tokenizer.h
        ../src/lisp/tokenizer/lisp_tokenizer.cpp
        ../src/lisp/tokenizer/lisp_parser_error.h
        ../src/lisp/tokenizer/lisp_tokenizer.cpp
        ../src/lisp/types.h
        ../src/tools/tools.cpp
        ../src/tools/location.h
        ../src/tools/rectangle.h
        ../src/lisp/tokenizer/lisp_syntax_checker.cpp
        ../src/lisp/Evaluator.cpp
        ../src/lisp/Evaluator.h
        ../src/lisp/EvalError.h
        ../src/lisp/environment.cpp
        ../src/lisp/environment.h
        ../src/lisp/DefaultEnvironment.cpp
        ../src/lisp/DefaultEnvironment.h
        test_tools.h
        ../src/lisp/NumericOperation.h
        ../src/lisp/native/eq.h
        ../src/lisp/ArgumentError.h
        ../src/lisp/NotImplementedError.h
        TestBase.h
)

set(TESTS
        test_lisp_comparison
        test_lisp_lambda
        test_lisp_if
        test_lisp_setq
        test_lisp_quote
        test_lisp_progn
        test_lisp_equality
        test_lisp_evaluator
        test_lisp_tokenizer
        test_lisp_parser
        test_location
)

foreach(test ${TESTS})
    add_executable(${test} ${test}.cpp ${ADDITIONAL_DEPS})
    target_link_libraries(${test} PRIVATE Qt6::Test Qt6::Core)
    add_test(NAME ${test} COMMAND ${test})
endforeach()