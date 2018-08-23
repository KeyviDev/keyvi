#!/usr/bin/env bash
set -ev

cd /io

cd python
python setup.py build --mode $CONF
python setup.py install --user
py.test tests
py.test integration-tests
cd ..
