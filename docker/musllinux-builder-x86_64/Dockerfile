FROM quay.io/pypa/musllinux_1_2_x86_64@sha256:37f0e89f9c6e66cde0807a6ca55967a8a7416b0012f0f4a7cd3054d00f79d360

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
