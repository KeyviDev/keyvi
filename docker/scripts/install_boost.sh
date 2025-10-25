#!/bin/sh
set -eu

BOOST_MAJOR=1
BOOST_MINOR=88
BOOST_PATCH=0

BOOST_URL="https://archives.boost.io/release/${BOOST_MAJOR}.${BOOST_MINOR}.${BOOST_PATCH}/source/boost_${BOOST_MAJOR}_${BOOST_MINOR}_${BOOST_PATCH}.tar.gz"
BOOST_FALLBACK_URL="https://sourceforge.net/projects/boost/files/boost/${BOOST_MAJOR}.${BOOST_MINOR}.${BOOST_PATCH}/boost_${BOOST_MAJOR}_${BOOST_MINOR}_${BOOST_PATCH}.tar.gz"
BOOST_DIR="boost_${BOOST_MAJOR}_${BOOST_MINOR}_${BOOST_PATCH}"

echo "Downloading Boost from ${BOOST_URL} (fallback: ${BOOST_FALLBACK_URL})"

if curl -fsSL "${BOOST_URL}" -o boost.tar.gz; then
    echo "Downloaded Boost successfully."
else
    echo "Primary download failed. Switching to fallback..."
    curl -fsSL "${BOOST_FALLBACK_URL}" -o boost.tar.gz
fi

echo "Extracting boost.tar.gz ..."
tar xzf boost.tar.gz

cd "${BOOST_DIR}"

echo "Bootstrapping Boost..."
./bootstrap.sh --with-libraries=system,program_options,iostreams,filesystem,test,interprocess,sort,container,process,format,lexical_cast

# Use nproc if available, otherwise default to 1
if command -v nproc >/dev/null 2>&1; then
    JOBS=$(nproc)
else
    JOBS=1
fi

echo "Building and installing Boost with ${JOBS} jobs..."
./b2 -j"${JOBS}" -d0 --prefix=/usr/local/ install

cd ..
rm -rf "${BOOST_DIR}" boost.tar.gz

echo "Boost ${BOOST_MAJOR}.${BOOST_MINOR}.${BOOST_PATCH} installed successfully."
