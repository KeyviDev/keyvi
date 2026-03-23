FROM quay.io/pypa/musllinux_1_2_x86_64@sha256:5b6fe3ed82ff48748c5c528fc82e42674a5363890ed949923af65b0c5b5ef76c

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
