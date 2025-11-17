FROM quay.io/pypa/musllinux_1_2_x86_64@sha256:5898f782e8526c62c2ad081191e9f0a5725014688e59dfa99bafd51a6df994bd

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
