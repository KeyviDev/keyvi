FROM quay.io/pypa/manylinux_2_28_aarch64@sha256:f701d22269f1b22e194aa561e2815214d5f5288049075a1ebd7aba0993f7c270

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
