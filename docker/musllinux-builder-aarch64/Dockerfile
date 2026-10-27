FROM quay.io/pypa/musllinux_1_2_aarch64@sha256:1db347533f8a7ea39aa20b0203f4e4af00630cdb621216a2b43771523c9983c5

RUN \
    apk upgrade --no-cache && \
    apk add --no-cache procps bzip2-dev snappy-dev python3-dev zstd-dev xxhash-dev openssl-dev;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
