$ErrorActionPreference = "Stop"

# Use Clang-cl compiler for MSVC ABI compatibility
$env:CC = "clang-cl"
$env:CXX = "clang-cl"

# Check for vcpkg
$VCPKG_ROOT = ""
if (Test-Path "vcpkg\scripts\buildsystems\vcpkg.cmake") {
    $VCPKG_ROOT = "$PSScriptRoot/vcpkg"
} elseif (Test-Path "D:/dev/cpp/vcpkg/scripts/buildsystems/vcpkg.cmake") {
    $VCPKG_ROOT = "D:/dev/cpp/vcpkg"
} else {
    Write-Host "vcpkg not found. Cloning vcpkg into local directory..." -ForegroundColor Cyan
    git clone https://github.com/Microsoft/vcpkg.git
    Write-Host "Bootstrapping vcpkg..." -ForegroundColor Cyan
    .\vcpkg\bootstrap-vcpkg.bat
    $VCPKG_ROOT = "$PSScriptRoot/vcpkg"
}

Write-Host "Configuring project with CMake and Clang..." -ForegroundColor Cyan

# Configure step (pointing to vcpkg toolchain)
cmake -B build -S . -G Ninja -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" -DCMAKE_BUILD_TYPE=Debug -DVCPKG_TARGET_TRIPLET=x64-windows-static

Write-Host "Building project..." -ForegroundColor Cyan
# Build step
cmake --build build

Write-Host "Build complete!" -ForegroundColor Green
