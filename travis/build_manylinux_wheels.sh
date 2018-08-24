#!/usr/bin/env bash
#set -ev
#
#cd /keyvi/python/
#
##case "${PYTHON_VERSION}" in
##  27)
##      PYBIN=/opt/python/cp27-cp27mu/bin
##      ;;
##  33)
##      PYBIN=/opt/python/cp33-cp33m/bin
##      ;;
##  34)
##      PYBIN=/opt/python/cp34-cp34m/bin
##      ;;
##  35)
##      PYBIN=/opt/python/cp35-cp35m/bin
##      ;;
##  36)
##      PYBIN=/opt/python/cp36-cp36m/bin
##      ;;
##  37)
##      PYBIN=/opt/python/cp37-cp37m/bin
##      ;;
##  pypy2)
##      echo "pypy2 is not support at the moment, see: https://github.com/pypa/manylinux/issues/38"
##      ;;
##  *)
##      echo "PYTHON_VERSION not set"
##esac
#
#PYBIN=/opt/python/${PYTHON_PATH}/bin
#
#${PYBIN}/pip install -r requirements.txt
#
#${PYBIN}/python setup.py bdist_wheel
#
## Bundle external shared libraries into the wheels
#for whl in dist/*.whl; do
#    auditwheel show $whl
#    auditwheel repair $whl -w wheelhouse/
#done
#


set -ex

PYBIN=/opt/python/${PYTHON_PATH}/bin

cd /io/python/

"${PYBIN}/pip" install -r requirements.txt

${PYBIN}/python setup.py bdist_wheel -d dist


# Bundle external shared libraries into the wheels
for whl in dist/*.whl; do
    auditwheel repair $whl -w wheelhouse/
done


"${PYBIN}/pip" install keyvi --no-index -f wheelhouse/

cd ~

${PYBIN}/py.test /io/python/tests/
${PYBIN}/py.test /io/python/integration-tests/
