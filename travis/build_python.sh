#!/usr/bin/env bash
set -ex

cd /io

pyenv global $PYTHON_VERSION

cd python

pip install -r requirements.txt

python setup.py build --mode $CONF
python setup.py install --user
py.test tests
py.test integration-tests
cd ..
