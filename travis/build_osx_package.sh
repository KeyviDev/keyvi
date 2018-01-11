#!/usr/bin/env bash
set -ev

mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=release -DZLIB_ROOT=/usr/local/opt/zlib ..
make -j 4
./unit_test_all
cd ..

# use python from pyenv
PYENV_ROOT="$HOME/.pyenv"
PATH="$PYENV_ROOT/bin:$PYENV_ROOT/shims:$PATH"

export TMPDIR=/Volumes/ram-disk

cd python
python setup.py bdist_wheel -d wheelhouse --zlib-root /usr/local/opt/zlib

# check that static linkage worked by uninstalling libraries
brew remove zlib
brew remove snappy

sudo -H pip install wheelhouse/*.whl
py.test tests
cd ..
