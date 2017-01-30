# pykeyvi

This is the python extension.

## Quick

Precompiled binary wheels are available for OS X and Linux on [PyPi](https://pypi.python.org/pypi/pykeyvi). To install use:

    pip install pykeyvi

## From source

Ensure you have the C++ dependencies installed along with python packages defined in `requirements.txt`:

Then build/install a python package:

    python setup.py build
    python setup.py install


## Develop

pykeyvi uses [cython](http://cython.org/) and [autowrap](https://github.com/uweschmitt/autowrap). All source files can be found in the src folder. The files pykeyvi.cpp and pykeyvi.pyx are generated on the fly during build.
