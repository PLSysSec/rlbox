# Using/Building docs

You can view the pre-built docs checked in to the repo in the docs folder.
Alternatively, you can build these yourself with the steps listed below.

1. Go through setup listed in main [README](https://github.com/PLSysSec/rlbox_sandboxing_api/blob/master/README.md). first

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
    cmake -S . -B ./build --parallel
    cmake --build ./build --parallel --target docs
    ```

4. Open the doc file in your favorite browser:

   On Ubuntu:

    ```bash
    firefox `pwd`/docs/sphinx/index.html
    ```
