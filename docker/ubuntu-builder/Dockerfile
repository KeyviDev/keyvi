FROM ubuntu:16.04

# install packages required by pyenv
RUN set -ex \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y git  make build-essential libssl-dev zlib1g-dev libbz2-dev \
    libreadline-dev libsqlite3-dev \
    wget curl llvm libncurses5-dev libncursesw5-dev xz-utils tk-dev libffi-dev liblzma-dev \
    && rm -rf /var/lib/apt/lists/*

# install packages required by keyvi
RUN set -ex \
    && apt-get update \
    && apt-get install -y cmake g++ libboost-all-dev libsnappy-dev libzzip-dev zlib1g-dev clang clang-format-6.0 doxygen \
    && rm -rf /var/lib/apt/lists/*

RUN update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-6.0 1000

# install pyenv
RUN curl -L https://github.com/pyenv/pyenv-installer/raw/master/bin/pyenv-installer | bash
ENV PATH "/root/.pyenv/shims/:/root/.pyenv/bin:$PATH"

# install python versions
RUN pyenv install 2.7.15
RUN pyenv install 3.4.5
RUN pyenv install 3.5.6
RUN pyenv install 3.6.6
RUN pyenv install 3.7.0
RUN pyenv install pypy2.7-6.0.0
RUN pyenv install pypy3.5-6.0.0

# install rust
RUN curl https://sh.rustup.rs -sSf | sh -s -- --default-toolchain stable -y
ENV PATH "~/.cargo/bin:$PATH"
