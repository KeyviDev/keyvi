FROM quay.io/pypa/musllinux_1_2_x86_64@sha256:04e877afa939007f6784f9076d2aa797d1156f31281cfe19f203945d7b624970

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
