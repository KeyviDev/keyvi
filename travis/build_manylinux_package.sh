#!/usr/bin/env bash
set -ev

docker run -e PYTHON_PATH --rm -v `pwd`:/keyvi $DOCKER_IMAGE /keyvi/travis/build_manylinux_wheels.sh


pyenv global $PYTHON_VERSION
pip install -r python/requirements.txt

pip install python/wheelhouse/keyvi*.whl
py.test python/tests/
py.test python/integration-tests/
