FROM quay.io/pypa/manylinux_2_28_x86_64@sha256:fe662034b57c4351087b0d0f09a11b7382ac68636f5c4c05a9179f8f5e90c2e7

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
