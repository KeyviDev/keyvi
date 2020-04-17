#!/usr/bin/env bash
set -x

python -m pip install --user wheel twine --upgrade

cd python
if [ -n "$(ls -A wheelhouse)" ]; then
    python -m twine upload --config-file ../travis/pypirc -u $PYPI_USERNAME -p $PYPI_PASSWORD wheelhouse/* || travis_terminate 1;
fi
