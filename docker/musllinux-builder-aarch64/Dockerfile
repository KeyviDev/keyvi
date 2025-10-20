FROM quay.io/pypa/musllinux_1_2_aarch64@sha256:5ebf27586ed7e77f54b41a3c4cecd8e271e62b2d30917a951f2e3b80201774e8

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
