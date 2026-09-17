FROM quay.io/pypa/musllinux_1_2_x86_64@sha256:8f7d1c70a5cfb088d9a9609a2671d7a1ac7cb46ccb19e7ac078ba9dbf2552068

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
