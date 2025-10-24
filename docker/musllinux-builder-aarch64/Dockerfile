FROM quay.io/pypa/musllinux_1_2_aarch64@sha256:3fe1e22f19f9727b22bd998f7006f461c3c351e58c128c857300b0b0a428af3c

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
