FROM quay.io/pypa/manylinux_2_28_aarch64@sha256:bbb43f784ce78a454a9e3351b862a12e7ad5c30972e8aeba092da678918bbba9

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
