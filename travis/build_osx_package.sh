#!/usr/bin/env bash
set -x

mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=release -DZLIB_ROOT=/usr/local/opt/zlib ..
make -j 4 || travis_terminate 1;

./unit_test_all --log_level=test_suite || travis_terminate 1;
cd ..


export TMPDIR=/Volumes/ram-disk

cd python
python setup.py bdist_wheel -d wheelhouse --zlib-root /usr/local/opt/zlib

# check that static linkage worked by uninstalling libraries
brew remove zlib
brew remove snappy

python -m pip install --user wheelhouse/*.whl
python -m pytest tests || travis_terminate 1;
python -m pytest integration-tests || travis_terminate 1;
cd ..
