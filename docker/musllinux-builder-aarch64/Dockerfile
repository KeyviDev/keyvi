FROM quay.io/pypa/musllinux_1_2_aarch64@sha256:d0b2cbe2cbc19b1cca501b298e1d0cfc10b7d7c7d75744c01aaa953f4cb14c4a

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
