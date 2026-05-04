FROM quay.io/pypa/musllinux_1_2_aarch64@sha256:19686ad406e671a3b98175e787978e69c9d440b027fa89fe46f9f98a954cd4bd

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
