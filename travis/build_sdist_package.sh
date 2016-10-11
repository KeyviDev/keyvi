#!/usr/bin/env bash
set -ev

cd pykeyvi
python setup.py sdist -d wheelhouse
sudo pip install wheelhouse/*.tar.gz
py.test tests
cd ..
