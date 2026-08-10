FROM quay.io/pypa/musllinux_1_2_aarch64@sha256:23371d5e69a9c90f17d8b36571204c7164c8fac0685725d05696aa8fd607f43b

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
