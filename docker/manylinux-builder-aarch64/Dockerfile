FROM quay.io/pypa/manylinux_2_28_aarch64@sha256:a030cc37f6ebf5ae85997978a7cee996235cd0f58c8a5ef9360da3521267dffb

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
