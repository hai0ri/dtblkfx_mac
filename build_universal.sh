#!/bin/bash
set -e

# Directory setup
PROJECT_ROOT="$(pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"
VCPKG_ROOT="${PROJECT_ROOT}/modules/vcpkg"

# Ensure vcpkg is bootstrapped
if [ ! -f "${VCPKG_ROOT}/vcpkg" ]; then
    echo "Bootstrapping vcpkg..."
    "${VCPKG_ROOT}/bootstrap-vcpkg.sh"
fi

# Install dependencies for both architectures
echo "Installing dependencies..."
"${VCPKG_ROOT}/vcpkg" install fftw3:x64-osx
"${VCPKG_ROOT}/vcpkg" install fftw3:arm64-osx

# Function to build for a specific architecture
build_arch() {
    VCPKG_ARCH=$1
    OSX_ARCH=$2
    
    echo "Building for ${OSX_ARCH} (vcpkg: ${VCPKG_ARCH})..."
    
    ARCH_BUILD_DIR="${BUILD_DIR}/${OSX_ARCH}"
    mkdir -p "${ARCH_BUILD_DIR}"
    
    cmake -B "${ARCH_BUILD_DIR}" -S "${PROJECT_ROOT}" \
        -DCMAKE_TOOLCHAIN_FILE="${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" \
        -DVCPKG_TARGET_TRIPLET="${VCPKG_ARCH}-osx" \
        -DCMAKE_OSX_ARCHITECTURES="${OSX_ARCH}" \
        -DCMAKE_BUILD_TYPE=Release
        
    cmake --build "${ARCH_BUILD_DIR}" --config Release
}

# Build for both architectures
build_arch "x64" "x86_64"
build_arch "arm64" "arm64"

# Create Universal Binary
echo "Creating Universal Binary..."
UNIVERSAL_DIR="${BUILD_DIR}/universal"
mkdir -p "${UNIVERSAL_DIR}"

# Path to the built VST3 bundles
# Note: JUCE usually outputs to <build_dir>/DtBlkFx_artefacts/Release/VST3/DtBlkFx.vst3
# We need to verify the exact path after the first build, but this is the standard pattern.
VST3_PATH_X64="${BUILD_DIR}/x86_64/DtBlkFx_artefacts/Release/VST3/DtBlkFx_GUI.vst3"
VST3_PATH_ARM64="${BUILD_DIR}/arm64/DtBlkFx_artefacts/Release/VST3/DtBlkFx_GUI.vst3"
UNIVERSAL_VST3="${UNIVERSAL_DIR}/DtBlkFx_GUI.vst3"

# Copy the arm64 bundle as the base for the universal bundle
cp -R "${VST3_PATH_ARM64}" "${UNIVERSAL_VST3}"

# Lipo the binaries
BINARY_NAME="DtBlkFx_GUI"
LIPO_OUTPUT="${UNIVERSAL_VST3}/Contents/MacOS/${BINARY_NAME}"

lipo -create \
    "${VST3_PATH_X64}/Contents/MacOS/${BINARY_NAME}" \
    "${VST3_PATH_ARM64}/Contents/MacOS/${BINARY_NAME}" \
    -output "${LIPO_OUTPUT}"

echo "Universal Binary created at: ${UNIVERSAL_VST3}"
lipo -info "${LIPO_OUTPUT}"

# Code signing (ad-hoc)
echo "Code signing..."
codesign --force --deep --sign - "${UNIVERSAL_VST3}"

echo "Build complete!"
