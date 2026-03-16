# KalQlator

A simple spread sheet application which uses a Lisp-inspired language interpreter for its formulas.

![Demo](assets/kalqlator.gif)

This is a pure hobby project.

I created a similar project before using wxWidgets. You can find it here: https://github.com/pderichs/kalkulator.

## How to build

### Prerequisites

#### Libraries

Some development libraries are required to build this project. For lint jobs you also need `clang-tidy` and `cppcheck`.

For e.g. **Debian** based systems you can use

```bash
sudo apt install \                                                                                                                           2 ↵
  qt6-base-dev \
  qt6-base-dev-tools \
  qt6-tools-dev \
  qt6-tools-dev-tools \
  libgmp-dev libgmpxx4ldbl \
  clang-tidy cppcheck \
  libqt6test6 
```

For **Mac OS** you can use Homebrew to install the required libs:

```bash
brew install qt6 gmp llvm cppcheck
```

**Note:** For linting on Mac OS you might need to manually put run-clang-tidy from `llvm` to your PATH.

### Build commands

```bash
mise install # optional - tools can be installed manually as well
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Run Tests

```bash
cd build
ctest --output-on-failure
```

## Run Linter

```bash
mise run build-setup
mise run lint
```

## Run Linter and automatically fix issues

```bash
mise run build-setup
mise run lint-fix
```

## Built With

KalQlator uses Open Source Software:

- **[Qt](https://www.qt.io/)** – Cross-platform Application Framework
- **[GMP – GNU Multiple Precision Arithmetic Library](https://gmplib.org/)** – Arbitrary-precision arithmetic
- **[Material Icons and Symbols Font](https://fonts.google.com/icons)** – Icons

Please see their respective Licenses for more information.

## License

This project is licensed under the [GNU General Public License v3.0](https://www.gnu.org/licenses/gpl-3.0) – see the [LICENSE](LICENSE) file for details.

This project uses the Qt Framework (https://www.qt.io).
Qt modules used in this project are licensed under the GNU Lesser General
Public License v3.0 (LGPLv3).
See: https://www.gnu.org/licenses/lgpl-3.0.html

This project uses the GNU Multiple Precision Arithmetic Library (GMP) (https://gmplib.org/).
GMP is licensed under the GNU Lesser General Public License v3.0 (LGPLv3).
See: https://www.gnu.org/licenses/lgpl-3.0.html

This project uses Material Icons Font and Material Symbols Font (https://fonts.google.com/icons)  
by Google, licensed under the [Apache License 2.0](https://www.apache.org/licenses/LICENSE-2.0).