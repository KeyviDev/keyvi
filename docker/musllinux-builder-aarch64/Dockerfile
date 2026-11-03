FROM quay.io/pypa/musllinux_1_2_aarch64@sha256:4b8c72510cd25410a9a12535ea15028b043bfc58a886741066ed06c9c06d4d3e

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
