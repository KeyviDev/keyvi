#!/bin/bash

set -ev

# get pyenv if it isn't there already
if [ ! -d ~/.pyenv ]; then
  git clone https://github.com/yyuu/pyenv.git ~/.pyenv
fi

pyenv install --list

PYENV_ROOT="$HOME/.pyenv"
PATH="$PYENV_ROOT/bin:$PATH"
eval "$(pyenv init -)"

# Make sure the cache directory exists
PYTHON_BUILD_CACHE_PATH="${PYTHON_BUILD_CACHE_PATH:-$HOME/.pyenv_cache}"
mkdir -p "$PYTHON_BUILD_CACHE_PATH"

case "${PYTHON_VERSION}" in
  27)
      pyenv install 2.7.12
      pyenv global 2.7.12
      ;;
  33)
      pyenv install 3.3.6
      pyenv global 3.3.6
      ;;
  34)
      pyenv install 3.4.5
      pyenv global 3.4.5
      ;;
  35)
      pyenv install 3.5.2
      pyenv global 3.5.2
      ;;
  pypy2)
      pyenv install pypy-5.3.1
      pyenv global pypy-5.3.1
      ;;
  *)
      echo "PYTHON_VERSION not set"
esac

pyenv rehash
python --version

# print some information, useful for debugging
echo $PATH
export PATH=$PATH
which python
python -c "import distutils.util; print(distutils.util.get_platform())"

# install other python deps
pip install --upgrade pip
pip install --user -r pykeyvi/requirements.txt
if [ "$CC" = "gcc" ] && [ "$CONF" = "coverage" ]; then pip install coveralls-merge cpp-coveralls ; fi
