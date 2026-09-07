FROM quay.io/pypa/musllinux_1_2_x86_64@sha256:621f8004ed526a5a6bf6a866fb415ad8da54d59a991e50b3b69167c3a768a616

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
