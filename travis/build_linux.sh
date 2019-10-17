#!/usr/bin/env bash
set -ex

cd /io

df -h
env
ls -l /tmp
ulimit -n

mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=$CONF ..
make -j 4

./unit_test_all
