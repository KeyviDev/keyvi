FROM quay.io/pypa/musllinux_1_2_x86_64@sha256:2178554fe4aa2b7b0182b3d711939f3f56455b9a782d521941342cc5794a9880

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
