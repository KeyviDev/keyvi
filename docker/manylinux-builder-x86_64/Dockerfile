FROM quay.io/pypa/manylinux_2_28_x86_64@sha256:6b71f3eb976315642dc89ed5df0de5a0a7c21bcbdcbcfd6bec13a3679750cbe1

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
