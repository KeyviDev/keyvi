# keyvi: C++

### Pre-Requisites

#### Linux

In addition to a working gcc build environment, install the following libraries, e.g. using apt: boost (dev packages, at least version 1.54), snappy, zlib, cmake.

For example on Ubuntu (14.04 to 16.04) should install all the dependencies you need:

    apt-get install cmake cython g++ libboost-all-dev libsnappy-dev libzzip-dev python-stdeb zlib1g-dev

#### MAC

In addition to a working build setup (Xcode) install the following libraries using Homebrew:

    brew install boost
    brew install snappy
    brew install lzlib
    brew install cmake

Now you should be able to compile as explained above.

## Build

Use `cmake` to build keyvi executables along with unit tests.

Example:

    mkdir build_dir_<BUILD_TYPE>
    cd build_dir_<BUILD_TYPE>
    cmake -DCMAKE_BUILD_TYPE=<BUILD_TYPE> ..
    make

`<BUILD_TYPE>` can be `release`, `debug`, `coverage` or any other available by default in `cmake`

To run cpp unit tests just execute `unit_test_all` executable.

## Coding Rules

The cpp part of keyvi has to pass the [https://pypi.python.org/pypi/cpplint](cpplint) checks as well as [https://clang.llvm.org/docs/ClangFormat.html](clang-format) (5.0.0 at the moment, defined by our travis CI configuration). 

The configuration of both tools is part of the project. You can check your changes against the config using the `check-style.sh` script. For your convenience consider using a plugin for your IDE of choice (e.g. CppStyle for Eclipse).

Formatting is not done automatically for you but code neads to be cleanly pushed. 

The check must pass for all changed, including new, files. 
