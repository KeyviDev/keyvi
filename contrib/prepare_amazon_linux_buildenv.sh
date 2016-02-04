#!/usr/bin/env bash
sudo yum update -y
sudo yum install -y gcc48-c++.x86_64 gcc44.x86_64
sudo yum install -y zlib-devel
sudo yum install -y libicu-devel libicu
sudo yum install -y compat-libicu4
sudo yum install -y python27-Cython
sudo yum install -y git
sudo yum install -y scons cmake
sudo yum install -y libquadmath libquadmath-devel
sudo yum install -y libstdc++48-static libstdc++48-devel
sudo yum install -y rpmdevtools