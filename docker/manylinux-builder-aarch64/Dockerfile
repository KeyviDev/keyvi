FROM quay.io/pypa/manylinux_2_28_aarch64@sha256:ca1f9d96910e129d38f999644a1f211a6cdb4147da9f19157a49d72c5092453a

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
