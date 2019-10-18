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

 ulimit -Hn && ulimit -Sn
lsof | wc -l

./unit_test_all -l all
