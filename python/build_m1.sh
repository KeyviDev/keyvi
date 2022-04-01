#!/bin/bash

set -e

for PY_VERSION in "3.8.13" "3.9.11" "3.10.3";
do
   echo "${PY_VERSION}"
   pyenv install -s "${PY_VERSION}"
   pyenv virtualenv -f "${PY_VERSION}" keyvi-"${PY_VERSION}"
   pyenv local keyvi-"${PY_VERSION}"
   python --version

   pip install -r requirements.txt
   pip wheel . -w wheelhouse/ --no-deps -vvv
done