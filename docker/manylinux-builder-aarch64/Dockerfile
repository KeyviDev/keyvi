FROM quay.io/pypa/manylinux_2_28_aarch64@sha256:c46b924d373b6dcbfb4024b44041e803601f7d67d2bb6e68a9f78f486b95bd9d

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
