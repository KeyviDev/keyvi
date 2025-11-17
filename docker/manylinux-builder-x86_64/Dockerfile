FROM quay.io/pypa/manylinux_2_28_x86_64@sha256:339edb1936d9eb06f05d585c6f96c0ec0b8031e99d4d2b952675ac1b08809de7

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
