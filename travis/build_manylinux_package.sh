#!/usr/bin/env bash
set -ev

docker run -e "PYTHON_VERSION=$PYTHON_VERSION" --rm -v `pwd`:/keyvi $DOCKER_IMAGE /keyvi/travis/build_manylinux_wheels.sh

pip install python/wheelhouse/keyvi*.whl
py.test python/tests/
py.test python/integration-tests/
