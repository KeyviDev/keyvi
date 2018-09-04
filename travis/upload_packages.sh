#!/usr/bin/env bash
set -ex

pip install --user wheel twine --upgrade

cd python
if [ -n "$(ls -A wheelhouse)" ]; then
    twine upload --config-file ../travis/pypirc -u $PYPI_USERNAME -p $PYPI_PASSWORD wheelhouse/*
fi
