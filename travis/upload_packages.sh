#!/usr/bin/env bash
set -ev

cd pykeyvi
twine upload --config-file ../travis/pypirc -r pypitest -u $PYPI_USERNAME -p $PYPI_PASSWORD wheelhouse/*
