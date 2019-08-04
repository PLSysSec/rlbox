# RLBox API

RLBox sandboxing API (in C++ 17). See [docs](./docs/index.rst) for now.

## Building the docs

1. You need to install doxygen and then the sphinx and breathe python packages
   (e.g., with pip3).

   On Ubuntu:
```bash
sudo apt install doxygen python-sphinx python-breathe
```

   On Arch Linux:
```bash
sudo pacman -S doxygen python-sphinx python-breathe
```

2. Build the docs:

```bash
cmake -S . -B ./build
cmake --build ./build --target docs
```

3. Open the doc file in your favorite browser:

```bash
firefox `pwd`/build/docs/sphinx/index.html
```
