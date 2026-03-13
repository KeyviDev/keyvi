FROM quay.io/pypa/musllinux_1_2_x86_64@sha256:eb7262223279a09a1fd366b6984f8510a49a9471bec383bfe01ea23fd389a7e9

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
