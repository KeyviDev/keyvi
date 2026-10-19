FROM quay.io/pypa/musllinux_1_2_x86_64@sha256:915d73c75b4551f98fd7b12de655ef2e27419a6441ea07472b6e25e0d868d18d

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
