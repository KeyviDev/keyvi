FROM quay.io/pypa/manylinux_2_28_x86_64@sha256:0536c364004fa2a3c5041120b6fe35d84fc5bfe31f04c6a6304f13eac4a67b63

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
