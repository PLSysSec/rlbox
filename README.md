# RLBox API

[![Build Status](https://travis-ci.com/PLSysSec/rlbox_sandboxing_api.svg?branch=master)](https://travis-ci.com/PLSysSec/rlbox_sandboxing_api)

RLBox sandboxing API (in C++ 17). See the [online docs](https://docs.rlbox.dev). This code has been tested on 64-bit versions of Ubuntu and Mac OSX.

## Reporting security bugs

If you find a security bug, please do not create a public issue. Instead, file a security bug on bugzilla using the [following template link](https://bugzilla.mozilla.org/enter_bug.cgi?cc=tom%40mozilla.com&cc=nfroyd%40mozilla.com&cc=deian%40cs.ucsd.edu&cc=shravanrn%40gmail.com&component=Security%3A%20Process%20Sandboxing&defined_groups=1&groups=core-security&product=Core&bug_type=defect).

## Setup

Configure cmake to setup a build folder. This is needed to run tests or build
the docs.

```bash
cmake -S . -B ./build
```

## Running the tests

1. Go through Setup listed above first

2. To build and test:

```bash
cmake --build ./build --target check
```

Alternatively, you can run the build and test targets separately

```bash
cmake --build ./build
cmake --build ./build --target test
```

## Running the tests

Currently rlbox has been tested and should work with gcc-7 or later and clang-5 or later.

## Using/Building docs

You can view the pre-built docs checked in to the repo in the docs folder.
Alternatively, you can build these yourself with the steps listed below.

1. Go through Setup listed above first

2. You need to install doxygen and then the sphinx and breathe python packages
   (e.g., with pip3).

   On Ubuntu:
```bash
sudo apt install doxygen python-sphinx python-breathe
```

   On Arch Linux:
```bash
sudo pacman -S doxygen python-sphinx python-breathe
```

3. Build the docs:

```bash
cmake -S . -B ./build
cmake --build ./build --target docs
```

4. Open the doc file in your favorite browser:

```bash
firefox `pwd`/docs/sphinx/index.html
```

## Contributing Code/Docs

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
```

3. After making changes to the source, add any new required tests and run all
tests.

```bash
cmake --build ./build  --config Debug --target check
```

Alternatively, you can run the build and test targets separately

```bash
cmake --build ./build --config Debug
cmake --build ./build --config Debug --target test
```

4. Modify the docs as appropriate and rebuild docs as described earlier.
Rebuilding is required if you have added new APIs.

5. To make sure all code/docs are formatted with, we use clang-format.
Install using:

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

7. Submit the pull request.
