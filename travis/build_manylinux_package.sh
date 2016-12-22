#!/usr/bin/env bash
set -ev

docker run -e "PYTHON_VERSION=$PYTHON_VERSION" --rm -v `pwd`:/keyvi $DOCKER_IMAGE /keyvi/travis/build_manylinux_wheels.sh

pip install pykeyvi/wheelhouse/pykeyvi*.whl
py.test pykeyvi/tests/
