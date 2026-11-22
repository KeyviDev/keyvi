FROM quay.io/pypa/musllinux_1_2_aarch64@sha256:202c3e7e9a872d4e8d712621d3f63f02e4760a9b958679506841c452ad91f959

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
