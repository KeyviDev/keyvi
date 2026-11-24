FROM quay.io/pypa/musllinux_1_2_x86_64@sha256:43f81b5440015302cd0b0671d3d05d1961a2a0995cb7ff3d1725ce62990498a8

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
