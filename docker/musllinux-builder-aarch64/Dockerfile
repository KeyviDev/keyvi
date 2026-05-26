FROM quay.io/pypa/musllinux_1_2_aarch64@sha256:4bdbf0ed719added131878f2ff2acd4f65e344fc8cbda2f4a0fd4b33b34a0d41

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
