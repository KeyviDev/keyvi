FROM quay.io/pypa/manylinux_2_28_aarch64@sha256:2aefbfbe5682289ef6899d91a64386937f6215a8fe0c246986ca2ae58b6b93a3

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
