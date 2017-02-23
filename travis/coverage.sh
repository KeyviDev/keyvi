#!/usr/bin/env bash
set -ev

coveralls   -r . -b build/ -i keyvi \
            --gcov /usr/bin/gcov-4.8 --gcov-options '\-lp' \
            -e build/keyvi/3rdparty -e keyvi/3rdparty -e pykeyvi \
            -E '.*/tests/*.cpp' \
            -E '.*/src/cpp/keyvicompiler/keyvicompiler.cpp' \
            -E '.*/src/cpp/keyviinspector/keyviinspector.cpp' \
            -E '.*/src/cpp/keyvimerger/keyvimerger.cpp' \
            --dump keyvi.cov_report > /dev/null

# workaround for coverage measurement: symlink keyvi
cd pykeyvi/
ln -s ../keyvi keyvi
cd ..

coveralls   -r . -b pykeyvi/ -i pykeyvi \
            --gcov /usr/bin/gcov-4.8 --gcov-options '\-lp' \
            -e pykeyvi/keyvi/3rdparty -e build \
            -E '.*/autowrap_includes/autowrap_tools.hpp' \
            -E '.*/src/extra/attributes_converter.h' \
            -E '.*/pykeyvi.cpp' \
            --dump pykeyvi.cov_report_tmp > /dev/null

# workaround: remove 'pykeyvi' from source path before merge
sed s/"pykeyvi\/keyvi"/"keyvi"/g pykeyvi.cov_report_tmp > pykeyvi.cov_report

export COVERALLS_REPO_TOKEN=${COVERALLS_REPO_TOKEN}

coveralls-merge keyvi.cov_report pykeyvi.cov_report
