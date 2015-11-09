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

#### Manually Installing on MAC

    brew install boost --c++11
    brew install scons
    brew install snappy
    brew install lzlib
    brew install cmake
    
For python-keyvi:
    
    pip install cython

