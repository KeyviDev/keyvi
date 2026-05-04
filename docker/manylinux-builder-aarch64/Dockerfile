FROM quay.io/pypa/manylinux_2_28_aarch64@sha256:c269bed3fc0ba21aac08ff67bf1d657d4967a9c0582fc8880a0e302f8ca78bdd

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
