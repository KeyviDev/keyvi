FROM quay.io/pypa/manylinux_2_28_x86_64@sha256:b3d62db12aed878dae1964d15b9c4f7239a531a137191f6d22be9ca357ba3dec

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
