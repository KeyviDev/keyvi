FROM quay.io/pypa/manylinux_2_28_aarch64@sha256:47727fe2494ff3825368335c7a142476dffdaca1d3be4ed1f6ba86fec17c4c72

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
