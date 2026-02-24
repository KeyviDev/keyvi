FROM quay.io/pypa/musllinux_1_2_x86_64@sha256:06d59fd7e05c592569a7196b25fd36073bea31ee06b1a25ea7abb1029769763a

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
