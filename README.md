# RLBox

[![Build Status](https://travis-ci.com/PLSysSec/rlbox_sandboxing_api.svg?branch=master)](https://travis-ci.com/PLSysSec/rlbox_sandboxing_api)

[![Tests](https://github.com/PLSysSec/rlbox_sandboxing_api/actions/workflows/cmake.yml/badge.svg)](https://github.com/PLSysSec/rlbox_sandboxing_api/actions/workflows/cmake.yml)

RLBox is a toolkit to securely sandbox third-party libraries. RLBox allows
chosen libraries to be isolated either with standard sandboxing tools like
WebAssembly, Native Client, etc. or by running them in a separate OS process.
RLBox's API then ensures that all interactions with these isolated libraries are
secure, by marking any untrusted data as `tainted`. Additionally RLBox
automatically handles any ABI differences introduced by tools like WebAssembly,
Native Client.

This code has been tested on 64-bit versions of Linux, Mac OSX, and Windows.

A more detailed RLBox tutorial is available [here](https://docs.rlbox.dev).

## Reporting security bugs

RLBox is used in production Firefox. Please do not file any security-sensitive bugs on GitHub. Instead, file a security bug on bugzilla using the [following template link](https://bugzilla.mozilla.org/enter_bug.cgi?component=Security%3A%20RLBox&defined_groups=1&groups=core-security&product=Core&bug_type=defect).

## Using this library

The RLBox library is a header only library, so you can directly download this repo and use include the contents of `code/include/` in your application. On Linux/Mac machines, you can optionally install the headers as well with `make install`.

Support for cmake's `find_package` API is also included. See the example in `examples/hello-world-cmake`.

The RLBox sandboxing API additionally needs a sandboxing plugin specific to the sandboxing technology used (WebAssembly, Native Client, OS
processes, etc.). This plugin must be downloaded separately. See the [tutorial](https://docs.rlbox.dev) for more details.

## Running the tests

1. Setup a build folder and then build.

   ```bash
   cmake -S . -B ./build -DCMAKE_BUILD_TYPE=Release
   ```

   You can build with cmake:

   ```bash
   cmake --build ./build --config Release --parallel
   ```

   or with make (on Linux/Mac):

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

## Install the library (Linux/Mac only)

Configure the build with cmake in the same way that previous paragraph. Then simply run:

   ```bash
   cd build
   make install
   ```

If you want to disable building tests, you can add `-DBUILD_TESTING=OFF` when invoking cmake the first time. This will also remove the Catch2 dependency.

   ```bash
   cmake -S . -B ./build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF
   ```

## Contributing Code

1. To contribute code, it is recommended you install `clang-tidy` (version 12 or
later), `clang-format`, `cppcheck`, `iwyu` (version 0.16 or later) which the
build uses if available. Install using:

   On Ubuntu:

   ```bash
   sudo apt install clang-tidy clang-format iwyu cppcheck
   ```

   On Arch Linux:

   ```bash
   sudo pacman -S clang-tidy clang-format cppcheck # iwyu not available for arch
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

4. If you have clang-format installed, format code with the format-source
target:

   ```bash
   cmake --build ./build --target format-source
   ```

5. Submit the pull request!

## Building the docs

RLBox's provides two forms of documentation.

- Tutorials on using RLBox and writing RLBox plugins
- Code documentation using Doxygen

The tutorials are developed and built in the [rlbox-book](https://github.com/PLSysSec/rlbox-book) repo.

Instructions for building the code documentation is given below.

1. To build the code documentation, you will need `doxygen` and `graphviz`. Install
using:

   On Ubuntu:

   ```bash
   sudo apt install doxygen graphviz
   ```

2. Build the docs using the docs target

   ```bash
   cmake --build ./build --target docs
   ```

   or with make (on Linux/Mac):

   ```bash
   cd build && make docs
   ```

3. Open the file `./doxygen/html/index.html` in a web browser.
