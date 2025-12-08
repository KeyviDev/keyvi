FROM quay.io/pypa/musllinux_1_2_aarch64@sha256:cec3bb6db1a350507310311be1b9309cb7a925b2005c39324d46ada242b57a73

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
