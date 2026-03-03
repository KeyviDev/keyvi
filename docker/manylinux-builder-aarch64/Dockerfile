FROM quay.io/pypa/manylinux_2_28_aarch64@sha256:34368babc008d6405b4b74c77868d6a5d15b6010cb953574d1f2bb9184392a9b

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
