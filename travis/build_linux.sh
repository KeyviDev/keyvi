#!/usr/bin/env bash
set -ev

cd /io

mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=$CONF ..
make -j 4

./unit_test_all
