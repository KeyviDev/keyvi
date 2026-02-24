FROM quay.io/pypa/musllinux_1_2_aarch64@sha256:974f8e7672eb3e363d5291d2bb92f354d1ef65cab207151f3952ce3d9f2d7c62

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
