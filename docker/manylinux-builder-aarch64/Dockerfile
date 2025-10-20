FROM quay.io/pypa/manylinux_2_28_aarch64@sha256:202a1466ef81720b304986cdb98a057def76135259642ff011218a4eef4ae55e

RUN \
    yum -y update && \
    yum clean all && \
    yum -y install bzip2-devel snappy-devel xxhash-devel libzstd-devel python3-devel openssl-devel && \
    yum clean all;

COPY docker/scripts/ /scripts/
WORKDIR /scripts

RUN ./install_zlib.sh
RUN ./install_boost.sh
RUN ./install_cmake.sh
RUN ./install_ccache.sh
