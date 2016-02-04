#!/usr/bin/env bash
wget http://sourceforge.net/projects/boost/files/boost/1.60.0/boost_1_60_0.tar.bz2

tar xvfj boost_1_60_0.tar.bz2
cd boost_1_60_0
./bootstrap.sh
./b2 cflags="-fPIC" --reconfigure
./b2 cflags="-fPIC" --debug-building -a -d+2 -d+4 --build-dir $bd
./b2 cflags="-fPIC" -d+2 -d+4 --build-dir $bd install
./b2 install

