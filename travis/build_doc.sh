#!/usr/bin/env bash
set -ex

docker run --rm -v `pwd`:/io \
    -e TRAVIS_BUILD_NUMBER \
    -e TRAVIS_COMMIT \
    -e DOXYFILE='/io/keyvi/keyvi.Doxyfile' \
    -e GH_REPO_NAME='keyvi' \
    -e GH_REPO_REF='github.com/KeyviDev/keyvi.git' \
    -e GH_REPO_TOKEN \
    ${DOCKER_IMAGE} /io/travis/generateDocumentationAndDeploy.sh
