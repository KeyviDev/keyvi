FROM quay.io/pypa/musllinux_1_2_x86_64@sha256:e84dcb1fefe01a2927848f1bfc431cfe249f07cb77c9b2cc933096750bbd518d

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
