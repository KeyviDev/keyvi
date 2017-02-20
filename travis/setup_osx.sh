#!/usr/bin/env bash
set -ev

diskutil erasevolume HFS+ 'ram-disk' `hdiutil attach -nomount ram://6165430`
df -h

brew update
brew install snappy
sudo -H easy_install pip
sudo -H pip install wheel
