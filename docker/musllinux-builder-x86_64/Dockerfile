FROM quay.io/pypa/musllinux_1_2_x86_64@sha256:1d9d20e8ca11fde22429ff36593de92c250657c557d9c1366ba59eb431859afb

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
