ARG base_image
FROM $base_image

RUN yum -y update; yum clean all

RUN yum -y install  bzip2-devel \
                    snappy-devel \
                    python-devel && \
                    yum clean all

ENV ZLIB_MAJOR=1 ZLIB_MINOR=3

RUN curl https://zlib.net/zlib-${ZLIB_MAJOR}.${ZLIB_MINOR}.tar.gz --output zlib-${ZLIB_MAJOR}.${ZLIB_MINOR}.tar.gz && \
    tar -xvf zlib-${ZLIB_MAJOR}.${ZLIB_MINOR}.tar.gz && \
    cd zlib-${ZLIB_MAJOR}.${ZLIB_MINOR} && ./configure && make -j4 && make install && \
    cd .. && rm -rf zlib-${ZLIB_MAJOR}.${ZLIB_MINOR} zlib-${ZLIB_MAJOR}.${ZLIB_MINOR}.tar.gz

ENV BOOST_MAJOR=1 BOOST_MINOR=83 BOOST_PATCH=0
RUN curl -s -SL https://boostorg.jfrog.io/artifactory/main/release/${BOOST_MAJOR}.${BOOST_MINOR}.${BOOST_PATCH}/source/boost_${BOOST_MAJOR}_${BOOST_MINOR}_${BOOST_PATCH}.tar.gz | tar xz && \
    cd boost_${BOOST_MAJOR}_${BOOST_MINOR}_${BOOST_PATCH} && \
    ./bootstrap.sh --without-libraries=graph_parallel,python, && \
    ./b2 -d0 --prefix=/usr/local/ install && \
    cd .. && \
rm -rf boost_*

ENV CMAKE_MAJOR=3 CMAKE_MINOR=27 CMAKE_PATCH=7

RUN curl -sSL https://cmake.org/files/v${CMAKE_MAJOR}.${CMAKE_MINOR}/cmake-${CMAKE_MAJOR}.${CMAKE_MINOR}.${CMAKE_PATCH}.tar.gz | tar -xz && \
    cd cmake-${CMAKE_MAJOR}.${CMAKE_MINOR}.${CMAKE_PATCH} && ./bootstrap -- -DCMAKE_USE_OPENSSL=OFF && make -j4 && make install && \
    cd .. && rm -rf cmake-${CMAKE_MAJOR}.${CMAKE_MINOR}.${CMAKE_PATCH}
