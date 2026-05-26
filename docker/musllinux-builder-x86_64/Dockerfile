FROM quay.io/pypa/musllinux_1_2_x86_64@sha256:ea90b935dec127485141fcfbcc480b75b642e2b499d6b4544ad45ad081ec0571

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
