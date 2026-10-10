FROM quay.io/pypa/musllinux_1_2_x86_64@sha256:bcd644f5f08de4c286b4e8a99652d106a7e56881d8e0b3f92d471f05f5b60fd3

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
