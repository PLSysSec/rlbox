# RLBox API

[![Build Status](https://travis-ci.com/PLSysSec/rlbox_api_cpp17.svg?branch=master)](https://travis-ci.com/PLSysSec/rlbox_api_cpp17)

RLBox sandboxing API (in C++ 17). See the [online docs](https://docs.rlbox.dev).

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
cmake --build ./build --target all
cmake --build ./build --target test
```

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

1. To contribute code, it is recommended you treat warnings as errors during
development. Also, you probably want to use the debug build. To do this, adjust
your build settings as shown below

```bash
cmake -DCMAKE_BUILD_TYPE=Debug -DWERR=ON -S . -B ./build
```

2. After making changes to the source, add any new required tests and run all
tests as described earlier.

3. Modify the docs as appropriate and rebuild docs as described earlier.
Rebuilding is required if you have added new APIs.

4. To make sure all code/docs are formatted with, we use clang-tidy.
Install using:

   On Ubuntu:
```bash
sudo apt install clang-tidy
```
   On Arch Linux:
```bash
sudo pacman -S clang-tidy
```

5. Format code with the format-source target:
```bash
cmake --build ./build --target format-source
```

6. Submit the pull request.
