FROM quay.io/pypa/musllinux_1_2_aarch64@sha256:3f5a8f81851495eaf343395d35602977042aeb88e6616e4cabfcee717e141fd8

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
