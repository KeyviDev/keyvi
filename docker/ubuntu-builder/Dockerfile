FROM ubuntu:16.04

RUN set -ex \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y git  make build-essential libssl-dev zlib1g-dev libbz2-dev \
    libreadline-dev libsqlite3-dev \
    wget curl llvm libncurses5-dev libncursesw5-dev xz-utils tk-dev libffi-dev liblzma-dev \
    && rm -rf /var/lib/apt/lists/*

RUN set -ex \
    && apt-get update \
    && apt-get install -y cmake cython g++ libboost-all-dev libsnappy-dev libzzip-dev python-stdeb zlib1g-dev \
    && rm -rf /var/lib/apt/lists/*


RUN curl -L https://github.com/pyenv/pyenv-installer/raw/master/bin/pyenv-installer | bash

ENV PATH "/root/.pyenv/shims/:/root/.pyenv/bin:$PATH"

RUN pyenv install 2.7.15
RUN pyenv install 3.3.6
RUN pyenv install 3.4.5
RUN pyenv install 3.5.6
RUN pyenv install 3.6.6
RUN pyenv install 3.7.0
RUN pyenv install pypy-5.3.1

#RUN curl -s https://static.rust-lang.org/rustup.sh | sh -s -- --channel=stable


# install toolchain
RUN curl https://sh.rustup.rs -sSf | sh -s -- --default-toolchain stable -y
ENV PATH "$HOME/.cargo/bin:$PATH"

#To get started you need Cargo's bin directory ($HOME/.cargo/bin) in your PATH
#environment variable. Next time you log in this will be done automatically.
#
#To configure your current shell run source $HOME/.cargo/env
