FROM quay.io/pypa/musllinux_1_2_aarch64@sha256:83734eadb0172f639c75221229f42298568e63ea74e6d827ee3f91fb3a17e5b3

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
