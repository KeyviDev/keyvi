#!/usr/bin/env bash
set -ev

cd keyvi
scons -j 4 mode=$CONF
$CONF/dictionaryfsa_unittests/dictionaryfsa_unittests
cd ../pykeyvi
python setup.py build --mode $CONF
python setup.py install --user
py.test tests
cd ..
