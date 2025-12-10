FROM quay.io/pypa/manylinux_2_28_aarch64@sha256:38a57eb409aae39c75ba069fb51ad04ffe150d3c1547efe8cfdf47b3b7e3a60f

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
