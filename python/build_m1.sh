#!/bin/bash

set -e

for PYTHON_VERSION in "3.8.13" "3.9.11" "3.10.3";
do
   echo "${PYTHON_VERSION}"
   pyenv install -s "${PYTHON_VERSION}"
   pyenv virtualenv -f "${PYTHON_VERSION}" keyvi-"${PYTHON_VERSION}"
   pyenv local keyvi-"${PYTHON_VERSION}"
   python --version

   pip install -r requirements.txt

   pip wheel . -w wheelhouse/ --no-deps -vvv
   delocate-wheel wheelhouse/*.whl

   pip install keyvi --no-index -f wheelhouse/
   py.test -vv tests/
   py.test -vv integration-tests/

   delocate-listdeps wheelhouse/*.whl

   pyenv uninstall -f keyvi-"${PYTHON_VERSION}"
   rm .python-version

done
