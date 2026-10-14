FROM quay.io/pypa/musllinux_1_2_aarch64@sha256:83a69fa8383ba5ac2d4f01597b791ebf93269c9748270b51b7a54fbba0dcdd39

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
