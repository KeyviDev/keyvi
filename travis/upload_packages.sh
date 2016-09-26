#!/usr/bin/env bash
set -ev

cd pykeyvi
if [ -n "$(ls -A wheelhouse)" ]; then
    twine upload --config-file ../travis/pypirc -r pypitest -u $PYPI_USERNAME -p $PYPI_PASSWORD wheelhouse/*
fi
