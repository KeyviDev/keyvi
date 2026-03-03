FROM quay.io/pypa/musllinux_1_2_aarch64@sha256:79e71688208c86dde8ce8193b07a7c4180796dcd729bec8e65242856211daf8e

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
