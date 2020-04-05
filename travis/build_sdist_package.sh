#!/usr/bin/env bash
set -ex

pyenv global 3.5.7

cd /io/python

pip install -r requirements.txt

python setup.py sdist -d wheelhouse

pip uninstall -y autowrap
pip install wheelhouse/*.tar.gz
py.test tests
py.test integration-tests
cd ..
