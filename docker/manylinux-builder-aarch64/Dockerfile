FROM quay.io/pypa/manylinux_2_28_aarch64@sha256:c2b6acdad57fc8ab9fcca5766e108d4e453549d9cb64cabc173e579f3c701977

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
