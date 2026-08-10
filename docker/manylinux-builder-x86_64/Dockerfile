FROM quay.io/pypa/manylinux_2_28_x86_64@sha256:f854c50adf7b7a325bc4794316f3758d387a41d61f9e2ebca0f26c7dc8f761d4

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
