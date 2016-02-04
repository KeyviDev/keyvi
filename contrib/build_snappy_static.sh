#!/usr/bin/env bash
wget https://github.com/google/snappy/releases/download/1.1.3/snappy-1.1.3.tar.gz
tar xvfz snappy-1.1.3.tar.gz
cd snappy-1.1.3
./configure CFLAGS="-fPIC" CXXFLAGS="-fPIC"
export CFLAGS="-fPIC"
make
sudo make install
