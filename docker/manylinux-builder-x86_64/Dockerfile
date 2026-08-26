FROM quay.io/pypa/manylinux_2_28_x86_64@sha256:4dc41da7df20400310c80d162a2fe2d2c2f3d9734d8dec20f6b9843711618deb

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
