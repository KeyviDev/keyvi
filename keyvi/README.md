# C++ keyvi library

## Compile

You need scons, simple compile by typing 

    scons
    
That compiles the library in debug mode, to compile in release mode (required for the python extension):

    scons mode=release
    
## Package

To create a dpkg (debian/ubuntu package) run:

    scons mode=release debian


### Pre-Requisites

#### Linux

In addition to a working gcc build environment, install the following libraries, e.g. using apt: boost (dev packages), scons, snappy, zlib, cmake.

#### MAC

In addtion to a working build setup (Xcode) install the following libraries using homebrew:

    brew install boost --c++11
    brew install scons
    brew install snappy
    brew install lzlib
    brew install cmake
    
Now you should be able to compile as explained above.
    
For python-keyvi you need cython:
    
    pip install cython

