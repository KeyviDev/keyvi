FROM quay.io/pypa/musllinux_1_2_x86_64@sha256:08426b0bfb91d58c4a2e11a8c564d295df663077fc8be7fc14709e2243b73685

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
