#!/usr/bin/env bash
set -ex

PYBIN=/opt/python/${PYTHON_PATH}/bin

cd /io/python/

${PYBIN}/pip install -r requirements.txt

# Build
${PYBIN}/python setup.py bdist_wheel -d dist

# Bundle external shared libraries into the wheels
for wheel in dist/*.whl; do
    auditwheel repair ${wheel} -w wheelhouse/
done

# Install and test
${PYBIN}/pip install keyvi --no-index -f wheelhouse/

cd ~
${PYBIN}/py.test /io/python/tests/
${PYBIN}/py.test /io/python/integration-tests/
