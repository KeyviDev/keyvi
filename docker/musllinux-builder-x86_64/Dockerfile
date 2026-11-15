FROM quay.io/pypa/musllinux_1_2_x86_64@sha256:cf82db9d8dc8c5c2cb27f422208baf266ebc8ad70be4ab1bc79bcc8807a6432d

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
