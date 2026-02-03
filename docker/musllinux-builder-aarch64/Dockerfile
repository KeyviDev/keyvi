FROM quay.io/pypa/musllinux_1_2_aarch64@sha256:fa77504f376714e26da82eca725d91171d84b9b95a60e3c3ba8edc00bce3725e

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
