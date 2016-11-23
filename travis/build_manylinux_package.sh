#!/usr/bin/env bash
set -ev

docker run --rm -v `pwd`:/keyvi $DOCKER_IMAGE /keyvi/travis/build_manylinux_wheels.sh

sudo -H pip install pykeyvi/wheelhouse/pykeyvi*.whl
py.test pykeyvi/tests/
