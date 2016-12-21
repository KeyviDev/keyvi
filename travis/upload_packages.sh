#!/usr/bin/env bash
set -ev

# use python from pyenv
PYENV_ROOT="$HOME/.pyenv"
PATH="$PYENV_ROOT/bin:$PYENV_ROOT/shims:$PATH"

cd pykeyvi
if [ -n "$(ls -A wheelhouse)" ]; then
    twine upload --config-file ../travis/pypirc -u $PYPI_USERNAME -p $PYPI_PASSWORD wheelhouse/*
fi
