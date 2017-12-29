#!/usr/bin/env bash
set -ev

cd python
python setup.py sdist -d wheelhouse

#pip uninstall -y autowrap
#pip install wheelhouse/*.tar.gz
#py.test tests
#cd ..
