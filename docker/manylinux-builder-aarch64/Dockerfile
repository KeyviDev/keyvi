FROM quay.io/pypa/manylinux_2_28_aarch64@sha256:223fca33616ee845af356469bf72a4af0faa092574ffc3f948a26f5e4fd0b70c

RUN \
    yum -y update && \
    yum clean all && \
    yum -y install bzip2-devel snappy-devel xxhash-devel libzstd-devel python3-devel openssl-devel && \
    yum clean all;

COPY --from=scripts . /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
