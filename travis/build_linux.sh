#!/usr/bin/env bash
set -ev

mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=$CONF ..
make -j 4
./units_test_all

cd ../pykeyvi

which python
which pip
env

PYENV_ROOT="$HOME/.pyenv"
PATH="$PYENV_ROOT/bin:$PATH"
eval "$(pyenv init -)"

python setup.py build --mode $CONF
python setup.py install --user
py.test tests
cd ..
