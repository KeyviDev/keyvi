FROM quay.io/pypa/musllinux_1_2_x86_64@sha256:c564755a06228cc28fe8383fc5fde4468f6cea91b9d35a38eaf853018bb3b320

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
