#!/usr/bin/env bash
set -ev

# for now only check changed files, pull requests only
if [ "$TRAVIS_PULL_REQUEST" != "false" ]; then
    ./keyvi/check-style.sh
fi
