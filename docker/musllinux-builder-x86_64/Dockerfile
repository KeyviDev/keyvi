FROM quay.io/pypa/musllinux_1_2_x86_64@sha256:da5e634078865cff5b035cc87560adaa72193b835d4f8d258cf3c6d3a5dd494f

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
