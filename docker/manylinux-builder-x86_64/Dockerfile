FROM quay.io/pypa/manylinux_2_28_x86_64@sha256:a658c70bfc08efa30156ef84775cfea959662cf0be52abab50b3b36ea8906784

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
