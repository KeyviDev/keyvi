#!/bin/sh
set -eu

CCACHE_MAJOR=4
CCACHE_MINOR=11
CCACHE_PATCH=3

CCACHE_VERSION="${CCACHE_MAJOR}.${CCACHE_MINOR}.${CCACHE_PATCH}"
CCACHE_TAR="ccache-${CCACHE_VERSION}.tar.gz"
CCACHE_DIR="ccache-${CCACHE_VERSION}"
CCACHE_URL="https://github.com/ccache/ccache/archive/refs/tags/v${CCACHE_VERSION}.tar.gz"

# --- Helpers ---
if command -v nproc >/dev/null 2>&1; then
    JOBS=$(nproc)
else
    JOBS=1
fi

# --- Download & Extract ---
echo "Downloading ccache ${CCACHE_VERSION} from ${CCACHE_URL}"
curl -fsSL "${CCACHE_URL}" -o "${CCACHE_TAR}"

echo "Extracting ${CCACHE_TAR}..."
tar -xzf "${CCACHE_TAR}"

# --- Build & Install ---
cd "${CCACHE_DIR}"
mkdir build
cd build

echo "Configuring ccache..."
cmake -D ENABLE_TESTING=OFF \
      -D REDIS_STORAGE_BACKEND=OFF \
      -D CMAKE_BUILD_TYPE=Release ..

echo "Building with ${JOBS} jobs..."
make -j"${JOBS}"

echo "Installing ccache..."
make install

# --- Cleanup ---
cd ../..
rm -rf "${CCACHE_DIR}" "${CCACHE_TAR}"

# --- Symlinks ---
echo "Creating compiler symlinks for ccache..."
for bin in gcc g++ cc c++; do
    if [ ! -e "/usr/local/bin/$bin" ]; then
        ln -s /usr/local/bin/ccache "/usr/local/bin/$bin" || true
    fi
done

echo "ccache ${CCACHE_VERSION} installed successfully."
