#!/bin/sh
set -eu

CMAKE_MAJOR=4
CMAKE_MINOR=1
CMAKE_PATCH=2

CMAKE_VERSION="${CMAKE_MAJOR}.${CMAKE_MINOR}.${CMAKE_PATCH}"
CMAKE_TAR="cmake-${CMAKE_VERSION}.tar.gz"
CMAKE_DIR="cmake-${CMAKE_VERSION}"
CMAKE_URL="https://cmake.org/files/v${CMAKE_MAJOR}.${CMAKE_MINOR}/${CMAKE_TAR}"

# --- Helpers ---
if command -v nproc >/dev/null 2>&1; then
    JOBS=$(nproc)
else
    JOBS=1
fi

# --- Download & Extract ---
echo "Downloading CMake ${CMAKE_VERSION} from ${CMAKE_URL}"
curl -fsSL "${CMAKE_URL}" -o "${CMAKE_TAR}"

echo "Extracting ${CMAKE_TAR}..."
tar -xzf "${CMAKE_TAR}"

# --- Build & Install ---
cd "${CMAKE_DIR}"

echo "Bootstrapping CMake..."
./bootstrap --parallel="${JOBS}" --

echo "Building CMake with ${JOBS} jobs..."
make -j"${JOBS}"

echo "Installing CMake..."
make install

# --- Cleanup ---
cd ..
rm -rf "${CMAKE_DIR}" "${CMAKE_TAR}"

echo "CMake ${CMAKE_VERSION} installed successfully."
