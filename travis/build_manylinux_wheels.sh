#!/usr/bin/env bash
set -ev

cd /keyvi/pykeyvi/

case "${PYTHON_VERSION}" in
  27)
      PYBIN=/opt/python/cp27-cp27mu/bin
      ;;
  33)
      PYBIN=/opt/python/cp33-cp33m/bin
      ;;
  34)
      PYBIN=/opt/python/cp34-cp34m/bin
      ;;
  35)
      PYBIN=/opt/python/cp35-cp35m/bin
      ;;
  *)
      echo "PYTHON_VERSION not set"
esac

${PYBIN}/python setup.py bdist_wheel

# Bundle external shared libraries into the wheels
for whl in dist/*.whl; do
    auditwheel show $whl
    auditwheel repair $whl -w wheelhouse/
done
