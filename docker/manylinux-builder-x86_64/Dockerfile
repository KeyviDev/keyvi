FROM quay.io/pypa/manylinux_2_28_x86_64@sha256:e40c7e3b275203c1322cd0445cff9fbc56d482282bfff67bbebdbc630eeff6a3

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
