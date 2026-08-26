FROM quay.io/pypa/musllinux_1_2_aarch64@sha256:0a584de3e287f20b66978ea0f3f774f80ce09bb4ac2c6941d08e0d70d46e3bc3

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
