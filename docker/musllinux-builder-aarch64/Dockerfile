FROM quay.io/pypa/musllinux_1_2_aarch64@sha256:0f4895c3efd2ee586d8be7c79559f7904bf46a520c4d8a4681232d3cf812b7c7

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
