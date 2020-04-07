#!/usr/bin/env bash
set -ex

pyenv global 3.5.7

cd /io/python

python -m pip install -r requirements.txt

python setup.py sdist -d wheelhouse

python -m pip uninstall -y autowrap
python -m pip install wheelhouse/*.tar.gz
python -m pytest tests
python -m pytest integration-tests
cd ..
