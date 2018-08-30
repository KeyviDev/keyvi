#!/usr/bin/env bash
set -ex

pip install pip --upgrade
pip install wheel twine --upgrade

cd python
if [ -n "$(ls -A wheelhouse)" ]; then
    twine upload --config-file ../travis/pypirc -u $PYPI_USERNAME -p $PYPI_PASSWORD wheelhouse/*
fi
