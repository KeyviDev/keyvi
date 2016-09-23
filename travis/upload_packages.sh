#!/usr/bin/env bash
set -ev

cd pykeyvi
python setup.py sdist -d wheelhouse/
twine upload --config-file ../travis/pypirc -r pypitest -u $PYPI_USERNAME -p $PYPI_PASSWORD wheelhouse/*
