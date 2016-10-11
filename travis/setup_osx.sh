#!/usr/bin/env bash
set -ev

brew update
brew install snappy scons cmake
sudo easy_install pip
sudo pip install wheel
