#!/usr/bin/env bash

set -euxo pipefail

docker build . \
    -f Dockerfile \
    --build-arg base_image="quay.io/pypa/manylinux2014_x86_64" \
    -t keyvidev/manylinux-builder-x86_64

docker build . \
    -f Dockerfile \
    --build-arg base_image="quay.io/pypa/manylinux2014_aarch64" \
    -t keyvidev/manylinux-builder-aarch64

docker push keyvidev/manylinux-builder-x86_64
docker push keyvidev/manylinux-builder-aarch64
