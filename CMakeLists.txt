cmake_minimum_required(VERSION 3.31.6)
project(kalqlator)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

enable_testing()
add_subdirectory(tests)

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

find_library(GMP_LIB gmp REQUIRED)
find_library(GMPXX_LIB gmpxx REQUIRED)

add_executable(kalqlator src/main.cpp
        src/ui/MainWindow.h
        src/ui/MainWindow.cpp
        src/lisp/object.h
        src/lisp/parser/parser.h
        src/lisp/parser/parser.cpp
        src/lisp/environment.h
        src/lisp/native/calculation.h
        src/lisp/factories.h
        src/lisp/tokenizer/tokenizer.h
        src/lisp/tokenizer/tokenizer.cpp
        src/lisp/parser/parser_error.h
        src/lisp/tokenizer/tokenizer.cpp
        src/lisp/types.h
        src/tools/tools.cpp
        src/tools/rectangle.h
        src/lisp/tokenizer/syntax_checker.cpp
        src/lisp/Evaluator.cpp
        src/lisp/Evaluator.h
        src/lisp/environment.cpp
        src/lisp/UnknownSymbolError.h
        src/lisp/EvalError.h
        src/lisp/SyntaxError.h
        src/lisp/TypeError.h
        src/lisp/DefaultEnvironment.cpp
        src/lisp/DefaultEnvironment.h
        src/lisp/NumericOperation.h
        src/lisp/native/eq.h
        src/lisp/tools.h
        src/lisp/ArgumentError.h
        src/lisp/native/progn.h
        src/lisp/NotImplementedError.h
        src/lisp/native/car.h
        src/lisp/native/cdr.h
        src/lisp/native/greater.h
        src/lisp/native/list.h
        src/model/Document.cpp
        src/model/Document.h
        src/model/Sheet.cpp
        src/model/Sheet.h
        src/model/Cell.h
        src/model/TableLispEnvironment.cpp
        src/model/TableLispEnvironment.h
        src/ui/user_interface_tools.h
        src/ui/KalqlatorTableWidget.cpp
        src/ui/KalqlatorTableWidget.h
        src/events/CellChangedEvent.h
        src/model/LispObjectStringConverter.cpp
        src/model/LispObjectStringConverter.h
        src/ui/FormulaBar.cpp
        src/ui/FormulaBar.h
        src/ui/FormulaEditorDialog.cpp
        src/ui/FormulaEditorDialog.h
        src/events/CellEvent.h
        src/events/SelectedCellChangedEvent.h
        src/tools/FlagScope.h
        src/ui/KalqlatorTableCellItemDelegate.h
        src/model/functions/Cell.h
        src/events/NewCellReferenceEvent.h
        src/events/CellUpdateErrorEvent.h
        src/file/DocumentJsonSerializer.cpp
        src/file/DocumentJsonSerializer.h
        src/events/DocumentLoadedEvent.h
        src/messagebus/event_sink.h
        src/messagebus/event_dispatcher.h
        src/events/TableEnvironmentUpdateEvent.h
        src/model/TableContext.h
        src/model/CircularReferenceError.h
        src/model/CellError.h
        src/model/CellErrorType.h
        src/ui/TableErrorDelegate.h
        src/ui/TableCellTypes.h
        src/ui/KalqWorksheetPanel.cpp
        src/ui/KalqWorksheetPanel.h
        src/model/MoveSheetDirection.h
        src/events/SheetSelectionChangedEvent.h
        resources/icons.qrc
        src/events/SelectionChangedEvent.h
        src/model/SheetRegistry.h
        src/model/functions/TableCell.h
        src/events/TablePropertyResizedEvent.h
        src/ui/KalqSidePanel.cpp
        src/ui/KalqSidePanel.h
        src/ui/SearchDialog.cpp
        src/ui/SearchDialog.h
        src/model/search/SearchOptions.h
        src/model/search/SearchResultItem.h
        src/events/SearchEvent.h
        src/events/SearchResultEvent.h
        src/ui/SearchResultItemMetaType.h
        src/events/SelectSheetAndCellEvent.h
        src/ui/CellChangeCommand.cpp
        src/ui/CellChangeCommand.h
        src/ui/MacroEditorDialog.cpp
        src/ui/MacroEditorDialog.h
        src/model/triggers.h
        src/events/MacroErrorEvent.h
        src/events/MacroEditorErrorEvent.h
        src/lisp/gmp_tools.h
        src/viewmodel/SpreadsheetModel.cpp
        src/viewmodel/SpreadsheetModel.h
        src/events/SheetListUpdateRequestEvent.h
)

target_link_libraries(kalqlator
        PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        ${GMP_LIB} ${GMPXX_LIB}
)


if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(kalqlator PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wshadow
            -Wnon-virtual-dtor
            -Wold-style-cast
            -Wcast-align
            -Wunused
            -Woverloaded-virtual
            -Wnull-dereference
            -Wformat=2
    )
endif()
