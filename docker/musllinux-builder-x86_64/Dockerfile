FROM quay.io/pypa/musllinux_1_2_x86_64@sha256:dc96c1f69b8a6241bcc8175a8d89b1caf0dccf959ff111a2272c42747af38350

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
