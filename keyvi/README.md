# keyvi: C++

### Pre-Requisites

#### Linux

In addition to a working gcc build environment, install the following libraries, e.g. using apt: boost (dev packages, at least version 1.54), snappy, zlib, cmake.

For example on Ubuntu (> 16.04) should install all the dependencies you need:

    apt-get install cmake cython g++ libboost-all-dev libsnappy-dev python-stdeb zlib1g-dev

#### MAC

In addition to a working build setup (Xcode) install the following libraries using Homebrew:

    brew install boost
    brew install snappy
    brew install lzlib
    brew install cmake

Now you should be able to compile as explained above.

#### Windows (experimental)

The following procedure has been confirmed working with Visual Studio Community 2019, other versions or environments _should_ work, too.

 - install cmake
 - install boost for windows from https://sourceforge.net/projects/boost/files/boost-binaries/
 - build and install using cmake, zlib: https://zlib.net/zlib-1.2.11.tar.gz
 - build and install using cmake, snappy: https://github.com/google/snappy/archive/refs/tags/1.1.8.tar.gz

## Build

Use `cmake` to build keyvi executables along with unit tests.

Example:

    mkdir build_dir_<BUILD_TYPE>
    cd build_dir_<BUILD_TYPE>
    cmake -DCMAKE_BUILD_TYPE=<BUILD_TYPE> ..
    make

`<BUILD_TYPE>` can be `release`, `debug`, `coverage` or any other available by default in `cmake`

To run cpp unit tests just execute `unit_test_all` executable.

#### Windows (experimental)

example windows command (make it 1 line):

```
cmake
-DCMAKE_BUILD_TYPE=Release
-DBOOST_ROOT="C:\\local\\Boost"
-DBOOST_INCLUDEDIR="C:\\local\\Boost\\boost"
-DBOOST_LIBRARYDIR="C:\\local\\Boost\\lib64-msvc-14.2"
-DSnappy_INCLUDE_DIR="C:\Program Files (x86)\Snappy\include"
-DSnappy_LIBRARY="c:\Program Files (x86)\Snappy\lib\snappy.lib"
```

(zlib should automatically be found if installed at the default location)

to build on the commandline (in addition cmake creates Visual Studio configs):

```
cmake --build . --config Release
```

## Coding Rules

The cpp part of keyvi has to pass the [https://pypi.python.org/pypi/cpplint](cpplint) checks as well as [https://clang.llvm.org/docs/ClangFormat.html](clang-format) (5.0.0 at the moment, defined by our travis CI configuration). 

The configuration of both tools is part of the project. You can check your changes against the config using the `check-style.sh` script. For your convenience consider using a plugin for your IDE of choice (e.g. CppStyle for Eclipse).

Formatting is not done automatically for you but code neads to be cleanly pushed. 

The check must pass for all changed, including new, files. 
