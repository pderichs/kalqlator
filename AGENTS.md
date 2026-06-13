# AGENTS.md

This file provides guidance to coding agents when working with code in this repository.

## Project Overview

KalQlator is a C++20 spreadsheet application built with Qt 6 that uses a custom Lisp interpreter for formula evaluation and GMP for arbitrary-precision arithmetic.

## Build Commands

```bash
# Build
mkdir build && cd build
cmake ..
make -j$(nproc)

# Run all tests (from build directory)
cd build && ctest --output-on-failure

# Run a single test (from build directory)
cd build && ctest -R test_lisp_tokenizer --output-on-failure

# Lint (prepare-lint generates compile_commands.json, required by clang-tidy)
mise run prepare-lint && mise run lint

# Lint with auto-fix
mise run lint-fix

# Static analysis
mise run cppcheck

# Run all linters (clang-tidy + cppcheck)
mise run check

# Format code
mise run fmt
```

Tools are managed via mise (cmake 4.2.3, clang 21.1.8). Run `mise install` to set up the
pinned toolchain, then `mise run setup` to install the system dependencies (`clang-tidy`,
`cppcheck`) via apt.

## Architecture

The application follows an **MVVM pattern** with an **event-driven message bus**:

- **Model** (`src/model/`): `Document` → `Sheet` → `Cell` hierarchy. Cells store content strings, evaluated values, and dependency information. `TableLispEnvironment` bridges the Lisp evaluator with spreadsheet context (cell references, macros).
- **ViewModel** (`src/viewmodel/`): `SpreadsheetModel` adapts the model as a `QAbstractTableModel` for Qt's table view.
- **View** (`src/ui/`): Qt widgets — `MainWindow`, `KalqlatorTableWidget`, `FormulaBar`, `KalqWorksheetPanel` (sheet tabs), `KalqSidePanel`, `SearchDialog`, `MacroEditorDialog`.
- **Message Bus** (`src/messagebus/`): `event_dispatcher` broadcasts events; components implement `event_sink` to listen. Events are defined in `src/model/events/`, `src/viewmodel/events/`, and `src/ui/events/`.
- **Persistence** (`src/file/`): `DocumentJsonSerializer` serializes/deserializes documents to/from JSON.
- **Utilities** (`src/tools/`): shared helpers such as `tools`, `rectangle`, `location`, and `FlagScope`.

### Lisp Interpreter (`src/lisp/`)

The formula engine is a Lisp interpreter with this pipeline: **Tokenizer** → **Parser** → **Evaluator** + **Environment**.

- `tokenizer/` — lexical analysis (also has `syntax_checker` for formula bar validation)
- `parser/` — produces an AST of `LispObjectPtr` nodes
- `Evaluator` — tree-walking interpreter
- `environment` — scoped symbol/function binding
- `DefaultEnvironment` — registers all built-in functions
- `native/` — native function implementations (arithmetic, comparison, list ops)
- `object.h` — core type system using `std::variant` (numbers via GMP, strings, symbols, cons cells, lambdas, native functions)

### Key Integration Points

- `TableLispEnvironment` extends the Lisp environment with spreadsheet functions (`CELL`, `TABLECELL`) that resolve cell references and track dependencies.
- Cell dependency graph enables automatic recalculation with circular reference detection (`CircularReferenceError`).
- Undo/redo uses Qt's `QUndoStack` via `CellChangeCommand`.
- Documents persist as JSON via `DocumentJsonSerializer`.

## Testing

Tests use **Qt Test** framework. Each test is a separate executable registered with CTest. Tests live in `tests/` and primarily cover the Lisp interpreter (tokenizer, parser, evaluator, comparisons, lambdas, conditionals, etc.) and utility classes.

## Compiler Settings

Warnings are treated strictly: `-Wall -Wextra -Wpedantic -Wshadow -Wnon-virtual-dtor -Wold-style-cast` and more (Clang only). All source files must compile warning-free.
