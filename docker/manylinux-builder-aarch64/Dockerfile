FROM quay.io/pypa/manylinux_2_28_aarch64@sha256:1c428544d9db0567ae2d99ce72f3f754d12c0a60702064fd8ed5f12c50c4d19e

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
