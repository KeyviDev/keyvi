# pykeyvi

This is the python extension.

## Compile

Ensure you build the C++ extension in release mode. After that just use the setup.py

    python setup.py build
    python setup.py install

## Develop

pykeyvi uses cython and [autowrap](https://github.com/uweschmitt/autowrap). All source files can be found in the src 
folder. The files pykeyvi.cpp and pykeyvi.pyx are generated for autowrap and cython and are checked in for convenience. 
Cython and autowrap are therefore only required if you plan to make changes in the bindings.

After changing cython code run the autowrap.sh script to regenerate the bindings.