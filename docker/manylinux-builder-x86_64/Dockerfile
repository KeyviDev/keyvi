FROM quay.io/pypa/manylinux_2_28_x86_64@sha256:a7333f64cc4dd1b103e1c9e1f3415267391bdf773c892720dd28b887cefc0c26

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
