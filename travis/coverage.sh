#!/usr/bin/env bash
set -ev

# workaround for coverage measurement: symlink cpp source:
cd pykeyvi/src
ln -s ../../keyvi/src/cpp/ .
cd ../..

coveralls   -r keyvi -b keyvi --gcov /usr/bin/gcov-4.8 --gcov-options '\-lp' \
            -e 3rdparty -E '.*/tests/*.cpp' \
            --dump keyvi.cov_report > /dev/null
coveralls   -r pykeyvi -b pykeyvi --gcov /usr/bin/gcov-4.8 --gcov-options '\-lp \-s '"$PWD"'/pykeyvi/keyvi' \
            -E '.*3rdparty' -E '.*/pykeyvi.cpp' -E '.*autowrap.*' \
            --dump pykeyvi.cov_report --follow-symlinks > /dev/null

export COVERALLS_REPO_TOKEN=${COVERALLS_REPO_TOKEN}

coveralls-merge  keyvi.cov_report pykeyvi.cov_report
