FROM quay.io/pypa/musllinux_1_2_x86_64@sha256:7ccba63dbde224328824ae871c11bb4134c47244b1fd4a5dfd1b89967e1375d1

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
