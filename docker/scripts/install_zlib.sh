#!/bin/sh
set -eu

ZLIB_MAJOR=1
ZLIB_MINOR=4
ZLIB_PATCH=0

#ZLIB_VERSION="${ZLIB_MAJOR}.${ZLIB_MINOR}.${ZLIB_PATCH}"
ZLIB_VERSION="${ZLIB_MAJOR}.${ZLIB_MINOR}"
ZLIB_TAR="zlib-${ZLIB_VERSION}.tar.gz"
ZLIB_DIR="zlib-${ZLIB_VERSION}"
ZLIB_URL="https://zlib.net/fossils/${ZLIB_TAR}"

ZLIB_SHA256="02dd159783677a749c04adbdd02b55b8a2bcc7e054f51b76b549199a916ad584"

# --- Helpers ---
if command -v nproc >/dev/null 2>&1; then
    JOBS=$(nproc)
else
    JOBS=1
fi

# --- Download ---
echo "Downloading zlib ${ZLIB_VERSION} from ${ZLIB_URL}"

echo "ZLIB_URL=${ZLIB_URL}"
echo "ZLIB_TAR=${ZLIB_TAR}"

curl --fail --silent --show-error --location \
    --retry 5 \
    --retry-delay 2 \
    --output "${ZLIB_TAR}" \
    "${ZLIB_URL}"

# --- Verify ---
echo "Verifying checksum..."
echo "${ZLIB_SHA256}  ${ZLIB_TAR}" | sha256sum -c -

echo "Testing gzip archive..."
gzip -t "${ZLIB_TAR}"

# --- Extract ---
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
