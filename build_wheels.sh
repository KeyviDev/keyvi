#!/usr/bin/env bash
set -ev

cd /keyvi/pykeyvi/

PYBIN=/opt/python/cp27-cp27mu/bin

${PYBIN}/python setup.py bdist_wheel

# Bundle external shared libraries into the wheels
for whl in dist/*.whl; do
    auditwheel show $whl
    auditwheel repair $whl -w wheelhouse/
done

#${PYBIN}/pip install pytest
#${PYBIN}/pip install python_keyvi -f wheelhouse/
#${PYBIN}/py.test tests/
