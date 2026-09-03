FROM quay.io/pypa/musllinux_1_2_aarch64@sha256:8ce7977050de0d80febd9d390fcadd4d5af4fc8349553902e645679fb260c125

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
