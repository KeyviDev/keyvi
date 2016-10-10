#!/usr/bin/env bash
set -ev

cd keyvi
scons -j 4 mode=release
cd ..

cd pykeyvi
python setup.py bdist_wheel -d osx_wheel
sudo pip install osx_wheel/*.whl
py.test tests
cd ..
