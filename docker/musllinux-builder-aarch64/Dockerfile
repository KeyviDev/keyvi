FROM quay.io/pypa/musllinux_1_2_aarch64@sha256:cb7074bb2d1100de7db80d2d2a38471a1f58431999b42736c8c645b466810c54

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
