#!/usr/bin/env bash
set -ev

cd /io

pyenv global $PYTHON_VERSION

cd python
python setup.py build --mode $CONF
python setup.py install --user
py.test tests
py.test integration-tests
cd ..
