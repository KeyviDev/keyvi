#!/bin/bash

set -e

for PYTHON_VERSION in "3.8.13" "3.9.11" "3.10.3";
do
    echo "Building on Python: ${PYTHON_VERSION}"
    pyenv install -s "${PYTHON_VERSION}"
    
    PYENV_VENV="keyvi-${PYTHON_VERSION}"

    echo "Installing pyenv-venv: ${PYENV_VENV}"
    pyenv virtualenv -f "${PYTHON_VERSION}" "${PYENV_VENV}"
    pyenv local "${PYENV_VENV}"
    python --version

    echo "Installing keyvi python deps..."
    pip install -r requirements.txt

    echo "Removing existing build artifacts..."
    rm -rf _core.cpp _core.pyi _core.pyx _core_p.cpp .pytest_cache/ build/

    echo "Building binary wheels..."
    pip wheel . -w wheelhouse/ --no-deps -vvv
    delocate-wheel wheelhouse/*.whl

    echo "Binary wheel deps:"
    delocate-listdeps wheelhouse/*.whl

    pip install keyvi --no-index -f wheelhouse/
    py.test -vv tests/
    py.test -vv integration-tests/

    echo "Uninstalling pyenv-venv: ${PYENV_VENV}"
    pyenv uninstall -f "${PYENV_VENV}"
    rm .python-version

done
