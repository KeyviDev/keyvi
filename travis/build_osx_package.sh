#!/usr/bin/env bash
set -ev

cd keyvi
scons -j 4 mode=release
cd ..

cd pykeyvi
python setup.py bdist_wheel -d wheelhouse
sudo -H pip install wheelhouse/*.whl
py.test tests
cd ..
