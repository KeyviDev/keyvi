#!/usr/bin/env bash
set -ex

cd /io

export CONF=coverage
travis/build_linux.sh

export PYTHON_VERSION=2.7.15
travis/build_python.sh

pip install coveralls-merge cpp-coveralls --upgrade

coveralls   -r . -b build/ -i keyvi \
            --gcov-options '\-lp' \
            -E '.*/keyvi/3rdparty/.*' \
            -e python \
            -E '.*/keyvi/tests/.*' \
            -E '.*/keyvi/bin/.*' \
            --dump keyvi.cov_report > /dev/null

# workaround for coverage measurement: symlink keyvi
cd python/
ln -s ../../../keyvi src/cpp/keyvi
cd ..

coveralls   -r . -b python/ -i python \
            --gcov-options '\-lp' \
            -e python/src/cpp/keyvi/3rdparty \
            -E '.*/src/cpp/build-.*/.*' \
            -E '.*/autowrap_includes/autowrap_tools.hpp' \
            -E '.*/src/extra/attributes_converter.h' \
            -E '.*/_core.cpp' \
            --dump python.cov_report_tmp > /dev/null

# workaround: remove 'python' from source path before merge
sed s/"python\/src\/cpp\/keyvi"/"keyvi"/g python.cov_report_tmp > python.cov_report

coveralls-merge keyvi.cov_report python.cov_report
