FROM quay.io/pypa/musllinux_1_2_aarch64@sha256:fecc36f309839b485f04e36b6fd2272d98b84936e68122f3de9bd65c253e76dc

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
