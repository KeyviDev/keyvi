FROM quay.io/pypa/musllinux_1_2_x86_64@sha256:c5071dd790d06f34e8ef0a36310988d6a1ad77d3b743ba1495e09529145833d0

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
