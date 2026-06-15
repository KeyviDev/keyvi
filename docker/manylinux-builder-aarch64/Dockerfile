FROM quay.io/pypa/manylinux_2_28_aarch64@sha256:f926f192db8589bdfbc6d4af4820d8cb76661f1349ab136259b33b94f4606f05

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
