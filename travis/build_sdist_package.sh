#!/usr/bin/env bash
set -ev

# https://github.com/travis-ci/travis-ci/issues/8920
python -c "import fcntl; fcntl.fcntl(1, fcntl.F_SETFL, 0)"

cd python
python setup.py sdist -d wheelhouse

pip uninstall -y autowrap
pip install wheelhouse/*.tar.gz
py.test tests
py.test integration-tests
cd ..
