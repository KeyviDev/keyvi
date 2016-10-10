#!/usr/bin/env bash
set -ev

brew update
brew install snappy scons

brew uninstall boost
wget 'https://github.com/hendrik-cliqz/travis-homebrew-bottle/releases/download/boost-1.61-cxx11/boost-1.61.0_1.mavericks.bottle.1.tar.gz'
brew install boost-1.61.0_1.mavericks.bottle.1.tar.gz
