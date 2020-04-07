#!/usr/bin/env bash
set -ex

pyenv global ${PYTHON_VERSION}

cd /io/python

python -m pip install -r requirements.txt

python setup.py build --mode ${CONF}
python setup.py install --user
python -m pytest tests
python -m pytest integration-tests
