#!/usr/bin/env bash
set -ev

mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=$CONF ..
make -j 4
./unit_test_all

cd ../python
python setup.py build --mode $CONF
python setup.py install --user
py.test tests
cd ..
