FROM quay.io/pypa/musllinux_1_2_aarch64@sha256:70a81491cfd91b4ebfc4f1b0bcd8e5da04d78a104cb8dc799f0b071034343df4

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
