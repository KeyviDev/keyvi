#!/usr/bin/env bash
set -ex

pyenv global 2.7.16
pip install cpplint

cd /io
./keyvi/check-style.sh
