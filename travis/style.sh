#!/usr/bin/env bash
set -ex

pyenv global 3.9.0
pip install cpplint

cd /io
./keyvi/check-style.sh
