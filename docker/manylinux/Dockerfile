FROM quay.io/pypa/manylinux1_x86_64

RUN yum -y install  zlib-devel \
                    bzip2-devel \
                    snappy-devel \
                    python-devel && \
                    yum clean all

RUN wget http://sourceforge.net/projects/boost/files/boost/1.60.0/boost_1_60_0.tar.bz2 --no-check-certificate \
    && tar xvfj boost_1_60_0.tar.bz2 && cd boost_1_60_0 && ./bootstrap.sh && ./b2 install -j 8 && cd ../ \
    && rm -rf boost_1_60_0  boost_1_60_0.tar.bz2

RUN ln -s /usr/bin/cmake28 /usr/bin/cmake
