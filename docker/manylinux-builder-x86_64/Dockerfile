FROM quay.io/pypa/manylinux_2_28_x86_64@sha256:893d0c9d73a8262503fa32f40160aae0299c10cecfc04396c9802a712cab1831

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
