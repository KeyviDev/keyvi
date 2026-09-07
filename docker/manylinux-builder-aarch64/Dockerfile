FROM quay.io/pypa/manylinux_2_28_aarch64@sha256:ad74e53b713f3b07d8c889c526dc0c6500da9827b45e38739570875fef52e28f

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
