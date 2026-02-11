FROM quay.io/pypa/musllinux_1_2_aarch64@sha256:d2dd3d34f476da44cd154ff32456c3a2d0bfca9aa376f77cfbf8fbc4b53a234b

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
