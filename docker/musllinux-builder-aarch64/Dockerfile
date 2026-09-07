FROM quay.io/pypa/musllinux_1_2_aarch64@sha256:4dffcd49f0b6fc6928a49915f3cd939f973bbecbdfe96e1e7926b6049bc0bad5

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
