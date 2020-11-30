FROM quay.io/pypa/manylinux2014_x86_64

RUN yum -y update; yum clean all

RUN yum -y install  bzip2-devel \
                    snappy-devel \
                    python-devel && \
                    yum clean all

ENV ZLIB_MAJOR=1 ZLIB_MINOR=2 ZLIB_PATCH=11

RUN curl -sSL https://zlib.net/zlib-${ZLIB_MAJOR}.${ZLIB_MINOR}.${ZLIB_PATCH}.tar.gz | tar -xz && \
    cd zlib-${ZLIB_MAJOR}.${ZLIB_MINOR}.${ZLIB_PATCH} && ./configure && make -j4 && make install && \
    cd .. && rm -rf zlib-${ZLIB_MAJOR}.${ZLIB_MINOR}.${ZLIB_PATCH}

ENV BOOST_MAJOR=1 BOOST_MINOR=74 BOOST_PATCH=0
RUN curl -s -SL http://sourceforge.net/projects/boost/files/boost/${BOOST_MAJOR}.${BOOST_MINOR}.${BOOST_PATCH}/boost_${BOOST_MAJOR}_${BOOST_MINOR}_${BOOST_PATCH}.tar.gz | tar xz && \
    cd boost_${BOOST_MAJOR}_${BOOST_MINOR}_${BOOST_PATCH} && \
    ./bootstrap.sh --without-libraries=graph_parallel,python, && \
    ./b2 -d0 --prefix=/usr/local/ install && \
    cd .. && \
rm -rf boost_*

ENV CMAKE_MAJOR=3 CMAKE_MINOR=10 CMAKE_PATCH=1

RUN curl -sSL https://cmake.org/files/v${CMAKE_MAJOR}.${CMAKE_MINOR}/cmake-${CMAKE_MAJOR}.${CMAKE_MINOR}.${CMAKE_PATCH}.tar.gz | tar -xz && \
    cd cmake-${CMAKE_MAJOR}.${CMAKE_MINOR}.${CMAKE_PATCH} && ./bootstrap && make -j4 && make install && \
    cd .. && rm -rf cmake-${CMAKE_MAJOR}.${CMAKE_MINOR}.${CMAKE_PATCH}
