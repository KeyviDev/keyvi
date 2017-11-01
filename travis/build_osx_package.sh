#!/usr/bin/env bash
set -ev

mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=release ..
make -j 4
./unit_test_all
cd ..

# use python from pyenv
PYENV_ROOT="$HOME/.pyenv"
PATH="$PYENV_ROOT/bin:$PYENV_ROOT/shims:$PATH"

export TMPDIR=/Volumes/ram-disk

cd pykeyvi
python setup.py bdist_wheel -d wheelhouse
sudo -H pip install wheelhouse/*.whl
py.test tests
cd ..
