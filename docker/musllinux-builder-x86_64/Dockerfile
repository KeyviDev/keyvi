FROM quay.io/pypa/musllinux_1_2_x86_64@sha256:3e50cbd89a956bf76b142f3adbdc4ca050eed4b3832b1bb4b2607709c15dc209

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
