FROM quay.io/pypa/musllinux_1_2_aarch64@sha256:4d17f735392dad187e377cbbaa1612a821c5d8d4c367c2b22f9baee79fab85ab

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
