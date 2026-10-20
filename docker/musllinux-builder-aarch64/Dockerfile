FROM quay.io/pypa/musllinux_1_2_aarch64@sha256:75a9ccd28ccc0abda65e462133f6e324943354e4e4936b51ba7c2ddc98352f86

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
