#!/bin/bash

set -e

for PYTHON_VERSION in "3.8.13" "3.9.11" "3.10.3";
do
    echo "Building on Python: ${PYTHON_VERSION}"
    pyenv install -s "${PYTHON_VERSION}"

    echo "Installing pyenv-venv: keyvi-${PYTHON_VERSION}"
    pyenv virtualenv -f "${PYTHON_VERSION}" keyvi-"${PYTHON_VERSION}"
    pyenv local keyvi-"${PYTHON_VERSION}"
    python --version

    echo "Installing keyvi python deps..."
    pip install -r requirements.txt

    echo "Building binary wheels..."
    pip wheel . -w wheelhouse/ --no-deps -vvv
    delocate-wheel wheelhouse/*.whl

    echo "Binary wheel deps:"
    delocate-listdeps wheelhouse/*.whl

    pip install keyvi --no-index -f wheelhouse/
    py.test -vv tests/
    py.test -vv integration-tests/

    echo "Uninstalling pyenv-venv: keyvi-${PYTHON_VERSION}"
    pyenv uninstall -f keyvi-"${PYTHON_VERSION}"
    rm .python-version

done
