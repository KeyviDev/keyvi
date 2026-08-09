FROM quay.io/pypa/musllinux_1_2_aarch64@sha256:a0e24d12e51dd0e0fe55385692e9917ce438c3b3c56e0f3e3641eeb0cff2e4f6

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
