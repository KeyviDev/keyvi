FROM quay.io/pypa/musllinux_1_2_x86_64@sha256:c2e9ac14b07f524aedecf2fa9f3f8eb9ae5724cf015fe4b4885a25d2899cadf5

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
