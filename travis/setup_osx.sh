#!/usr/bin/env bash
set -ev

diskutil erasevolume HFS+ 'ram-disk' `hdiutil attach -nomount ram://6165430`
df -h

brew update
brew install zlib
brew install snappy
brew install pyenv
