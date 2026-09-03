FROM quay.io/pypa/manylinux_2_28_aarch64@sha256:1676a4f178dc6cf8a1d1e3b7e1d71fca891a466ebde72a33880ccda8af3383d8

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
