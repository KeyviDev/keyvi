#!/usr/bin/env bash
set -ex

diskutil erasevolume HFS+ 'ram-disk' `hdiutil attach -nomount ram://6165430`
df -h

brew update
brew install zlib
brew install snappy

brew upgrade pyenv

export PATH="${HOME}/.pyenv/shims/:/root/.pyenv/bin:${PATH}"

pyenv install ${PYTHON_VERSION}
pyenv global ${PYTHON_VERSION}

pip install -r python/requirements.txt
