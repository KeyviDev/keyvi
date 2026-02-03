FROM quay.io/pypa/manylinux_2_28_aarch64@sha256:6ff5add4c780323bbc5a28a1f0a17228250952f5a5d45e4a6f642c518c9c3747

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
