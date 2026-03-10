FROM quay.io/pypa/manylinux_2_28_aarch64@sha256:5f642720997cd8686b721ab6cb895e32d532ccc625657205e29fb6363ffec7e5

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
