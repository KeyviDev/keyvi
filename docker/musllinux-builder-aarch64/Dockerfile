FROM quay.io/pypa/musllinux_1_2_aarch64@sha256:599a193abc52766a19fe078ac47e8e3cf27466a08d3307dd467b4c51bb20b8a7

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
