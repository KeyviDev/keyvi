FROM quay.io/pypa/musllinux_1_2_x86_64@sha256:246a4cce4d216188935e80beba5bcfc3693b2dbc97058d906a9962b9cadc0b69

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
