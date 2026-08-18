FROM quay.io/pypa/musllinux_1_2_x86_64@sha256:b06e331e36f400fa1529b4dd2ee37016f2828422b2e5d054a66257fcf4535697

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
