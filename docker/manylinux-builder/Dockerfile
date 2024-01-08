ARG base_image
FROM $base_image

RUN \
    if which yum; then \
        yum -y update && \
        yum clean all && \
        yum -y install bzip2-devel snappy-devel python-devel && \
        if [ $AUDITWHEEL_ARCH != "aarch64" ]; then \
          yum -y install hiredis-devel libzstd-devel; \
        fi && \
        yum clean all; \
    elif which apk; then \
        apk upgrade --no-cache && \
        apk add --no-cache bzip2-dev snappy-dev python3-dev hiredis-dev zstd-dev; \
    else \
        echo "unsupported os"; \
        exit 1; \
    fi

ENV ZLIB_MAJOR=1 ZLIB_MINOR=3

RUN curl https://zlib.net/zlib-${ZLIB_MAJOR}.${ZLIB_MINOR}.tar.gz --output zlib-${ZLIB_MAJOR}.${ZLIB_MINOR}.tar.gz && \
    tar -xvf zlib-${ZLIB_MAJOR}.${ZLIB_MINOR}.tar.gz && \
    cd zlib-${ZLIB_MAJOR}.${ZLIB_MINOR} && ./configure && make -j`nproc` && make install && \
    cd .. && rm -rf zlib-${ZLIB_MAJOR}.${ZLIB_MINOR} zlib-${ZLIB_MAJOR}.${ZLIB_MINOR}.tar.gz

ENV BOOST_MAJOR=1 BOOST_MINOR=83 BOOST_PATCH=0
ENV BOOST_URL="https://boostorg.jfrog.io/artifactory/main/release/${BOOST_MAJOR}.${BOOST_MINOR}.${BOOST_PATCH}/source/boost_${BOOST_MAJOR}_${BOOST_MINOR}_${BOOST_PATCH}.tar.gz"
ENV BOOST_FALLBACK_URL="https://sourceforge.net/projects/boost/files/boost/${BOOST_MAJOR}.${BOOST_MINOR}.${BOOST_PATCH}/boost_${BOOST_MAJOR}_${BOOST_MINOR}_${BOOST_PATCH}.tar.gz"

RUN echo "downhload boost ${BOOST_URL}, fallback: ${BOOST_FALLBACK_URL}"
RUN curl -s -SL ${BOOST_URL} -o boost.tar.gz
RUN tar xzf boost.tar.gz 2> /dev/null || (echo "switching to fallback" && curl -s -SL ${BOOST_FALLBACK_URL} -o boost.tar.gz && tar xzf boost.tar.gz)
RUN cd boost_${BOOST_MAJOR}_${BOOST_MINOR}_${BOOST_PATCH} && \
    ./bootstrap.sh --without-libraries=graph_parallel,python, && \
    ./b2 -j`nproc` -d0 --prefix=/usr/local/ install && \
    cd .. && \
rm -rf boost_*

ENV CMAKE_MAJOR=3 CMAKE_MINOR=27 CMAKE_PATCH=7

RUN curl -sSL https://cmake.org/files/v${CMAKE_MAJOR}.${CMAKE_MINOR}/cmake-${CMAKE_MAJOR}.${CMAKE_MINOR}.${CMAKE_PATCH}.tar.gz | tar -xz && \
    cd cmake-${CMAKE_MAJOR}.${CMAKE_MINOR}.${CMAKE_PATCH} && ./bootstrap --parallel=`nproc` -- -DCMAKE_USE_OPENSSL=OFF && make -j`nproc` && make install && \
    cd .. && rm -rf cmake-${CMAKE_MAJOR}.${CMAKE_MINOR}.${CMAKE_PATCH}

ENV CCACHE_MAJOR=4 CCACHE_MINOR=8 CCACHE_PATCH=3

RUN \
    if [ $AUDITWHEEL_ARCH == "aarch64" ] && [ $AUDITWHEEL_POLICY == "manylinux2014" ]; then \
        echo "skiping ccache, not available for this platform"; \
    else \
        curl -sSL https://github.com/ccache/ccache/archive/refs/tags/v${CCACHE_MAJOR}.${CCACHE_MINOR}.${CCACHE_PATCH}.tar.gz | tar -xz && \
        cd ccache-${CCACHE_MAJOR}.${CCACHE_MINOR}.${CCACHE_PATCH} && mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Release .. && make -j`nproc` && make install && \
        cd ../.. && rm -rf ccache-${CCACHE_MAJOR}.${CCACHE_MINOR}.${CCACHE_PATCH}; \
    fi

RUN ln -s /usr/local/bin/ccache /usr/local/bin/gcc && \
    ln -s /usr/local/bin/ccache /usr/local/bin/g++ && \
    ln -s /usr/local/bin/ccache /usr/local/bin/cc && \
    ln -s /usr/local/bin/ccache /usr/local/bin/c++ && \
    ln -s /usr/local/bin/ccache /usr/local/bin/clang && \
    ln -s /usr/local/bin/ccache /usr/local/bin/clang++