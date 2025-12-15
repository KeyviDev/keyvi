FROM quay.io/pypa/musllinux_1_2_x86_64@sha256:2c03fc5694010eba11195eb79e4e2edc8611f620272e9619a165fa8c8bd1aef9

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
