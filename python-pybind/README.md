# python version of keyvi

This is the python extension using pybind11 bindings.

## Quick

Precompiled binary wheels are available for OS X and Linux on [PyPi](https://pypi.python.org/pypi/keyvi). To install use:

    pip install keyvi

## From source

Ensure you have the C++ dependencies installed (see the main [README](/README.md)).

Then build and install:

    pip install .

For development (editable install):

    pip install -e . -v

## Develop

The python binding uses [pybind11](https://pybind11.readthedocs.io/) and [scikit-build-core](https://scikit-build-core.readthedocs.io/). The C++ binding source files can be found in the `src` folder.
