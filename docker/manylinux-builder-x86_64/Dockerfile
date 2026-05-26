FROM quay.io/pypa/manylinux_2_28_x86_64@sha256:441c35fdc6ee809ff9260894f8468ab4fea8c15dc880f8700a3f81b7922c1cda

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
