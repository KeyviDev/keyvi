#!/usr/bin/env bash
set -ev

docker run --rm -v `pwd`:/keyvi $DOCKER_IMAGE /keyvi/travis/build_wheels.sh

pip install --user python_keyvi -f wheelhouse/
py.test tests/
