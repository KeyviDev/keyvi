#!/bin/sh
set -eu

ZLIB_MAJOR=1
ZLIB_MINOR=3
ZLIB_PATCH=1

ZLIB_VERSION="${ZLIB_MAJOR}.${ZLIB_MINOR}.${ZLIB_PATCH}"
ZLIB_TAR="zlib-${ZLIB_VERSION}.tar.gz"
ZLIB_DIR="zlib-${ZLIB_VERSION}"
ZLIB_URL="https://zlib.net/${ZLIB_TAR}"

# --- Helpers ---
if command -v nproc >/dev/null 2>&1; then
    JOBS=$(nproc)
else
    JOBS=1
fi

# --- Download & Extract ---
echo "Downloading zlib ${ZLIB_VERSION} from ${ZLIB_URL}"
curl -fsSL "${ZLIB_URL}" -o "${ZLIB_TAR}"

echo "Extracting ${ZLIB_TAR}..."
tar -xzf "${ZLIB_TAR}"

# --- Build & Install ---
cd "${ZLIB_DIR}"

echo "Configuring zlib..."
./configure

echo "Building zlib with ${JOBS} jobs..."
make -j"${JOBS}"

echo "Installing zlib..."
make install

# --- Cleanup ---
cd ..
rm -rf "${ZLIB_DIR}" "${ZLIB_TAR}"

echo "zlib ${ZLIB_VERSION} installed successfully."
