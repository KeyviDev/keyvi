FROM quay.io/pypa/manylinux_2_28_aarch64@sha256:fb4a43425ac4e996a2056ce50f0d3e79df8bf0fa6ef11cc4632d18adfe67a639

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
