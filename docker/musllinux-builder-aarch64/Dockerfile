FROM quay.io/pypa/musllinux_1_2_aarch64@sha256:6cdf1dca55fa6fb1e689e7516e465b51eed1f822167386d61102116c49235d6f

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
