FROM quay.io/pypa/musllinux_1_2_x86_64@sha256:0092b7d1019f3110cc691e4997e9838e7c9f086655643aac442f273ce90efdd0

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
