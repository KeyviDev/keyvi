#!/usr/bin/env bash
set -ev

brew update
brew install snappy scons
sudo -H easy_install pip
sudo -H pip install wheel
