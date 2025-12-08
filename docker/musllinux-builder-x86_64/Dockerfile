FROM quay.io/pypa/musllinux_1_2_x86_64@sha256:88980edefa5f2847cdb9a05fbd30875c0b38a5c8536b1fb77fdadb2a848de3e8

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
