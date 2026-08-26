FROM quay.io/pypa/manylinux_2_28_aarch64@sha256:29a6ceb78de5b00494e6e7456a72782a4cb54238de102e7491d15fe47789b4d1

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
