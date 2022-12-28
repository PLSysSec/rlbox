# RLBox

[![Tests](https://github.com/PLSysSec/rlbox_sandboxing_api/actions/workflows/cmake.yml/badge.svg)](https://github.com/PLSysSec/rlbox_sandboxing_api/actions/workflows/cmake.yml)

RLBox sandboxing framework. This code has been tested on 64-bit versions of Linux, Mac OSX, and Windows.

## Reporting security bugs

If you find a security bug, please do not create a public issue. Instead, file a security bug on bugzilla using the [following template link](https://bugzilla.mozilla.org/enter_bug.cgi?component=Security%3A%20RLBox&defined_groups=1&groups=core-security&product=Core&bug_type=defect).

## Using this library

RLBox is a general purpose sandboxing API that can be used to interact with
library sandboxed with different backends --- WebAssembly, Native Client, OS
processess, etc. Support for each backend is provided by a separate plugin that
must also be downloaded separately.

See the [online docs](https://rlbox.dev) for more details.

The RLBox library is a header only library, so you can directly download this repo and use include the contents of `code/include/` in your application. On Linux/Mac machines, you can optionally install the headers as well with `make install`.

Support for cmake's `find_package` API is also included (see [this example](https://github.com/PLSysSec/rlbox-book/blob/main/src/chapters/examples/noop-hello-example/CMakeLists.txt)).

## Running the tests

1. Setup a build folder and then build.

   ```bash
   cmake -S . -B ./build -DCMAKE_BUILD_TYPE=Release
   ```

   You can build with cmake:

   ```bash
   cmake --build ./build --config Release --parallel
   ```

   or make:

   ```bash
   cd build && make -j
   ```

2. To test:

   With cmake:

   ```bash
   cd build && ctest -V
   ```

   or with make (on Linux/Mac):

   ```bash
   cd build && make test
   ```

   When running with ASAN and UBSAN:

   ```bash
   cd build && cmake -E env LSAN_OPTIONS=suppressions=../leak_suppressions.txt UBSAN_OPTIONS=suppressions=../ub_suppressions.txt ctest -V
   ```

Currently RLBox has been tested and should work with gcc-7 or later and
clang-5, Visual Studio 2019 (possibly previous versions as well) or later.  If
you are using other compilers/compiler versions (like mingw), these may also be
supported.  Simply run the test suite and check that everything passes.

If you want to disable building tests, you can add `-DBUILD_TESTING=OFF` when invoking cmake the first time. This will also remove the Catch2 dependency.

   ```bash
   cmake -S . -B ./build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF
   ```

## Install the library (Linux/Mac only)

The recommended use of RLBox is to make a copy of this library in your source tree.
However, if you want to install the library, you can do so by following the below steps.

Configure the build with cmake in the same way that previous paragraph. Then simply run:

   ```bash
   cd build
   make install
   ```

## Contributing Code

1. To contribute code, it is recommended you install clang-tidy which the build
uses if available. Install using:

   On Ubuntu:

   ```bash
   sudo apt install clang-tidy
   ```

   On Arch Linux:

   ```bash
   sudo pacman -S clang-tidy
   ```

2. It is recommended you use the dev mode for building during development. This
treat warnings as errors, enables clang-tidy checks, runs address sanitizer etc.
Also, you probably want to use the debug build. To do this, adjust your build
settings as shown below

   ```bash
   cmake -DCMAKE_BUILD_TYPE=Debug -DDEV=ON -S . -B ./build
   cd build
   make
   ```

3. After making changes to the source, add any new required tests and run all
tests (as described earlier).

4. If you don't already have clang-format installed, install it:

   On Ubuntu:

   ```bash
   sudo apt install clang-format
   ```

   On Arch Linux:

   ```bash
   sudo pacman -S clang-format
   ```

6. Format code with the format-source target:

   ```bash
   cmake --build ./build --target format-source
   ```

7. Submit the pull request!
