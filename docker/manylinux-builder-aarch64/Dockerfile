FROM quay.io/pypa/manylinux_2_28_aarch64@sha256:8b5f2b4e8c072ae5aefeb659f22c03e1ff46e6a82f154b6c904b106c87e65ff7

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
