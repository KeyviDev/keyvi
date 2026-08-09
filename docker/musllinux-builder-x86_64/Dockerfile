FROM quay.io/pypa/musllinux_1_2_x86_64@sha256:57c53870e12363dc7f707b4f62df2fe8309e773ecfc16891952d62e0c3488a92

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
