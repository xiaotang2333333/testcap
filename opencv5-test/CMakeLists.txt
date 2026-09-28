# Standalone consumer project for the overlay-ports/opencv5 port.
#
# Full local build (manifest mode, vcpkg installs opencv5 automatically on configure):
#   cmake -S . -B build ^
#     -DCMAKE_TOOLCHAIN_FILE=<vcpkg-root>\scripts\buildsystems\vcpkg.cmake ^
#     -DVCPKG_TARGET_TRIPLET=x64-windows
#   cmake --build build --config Release
#   build\Release\opencv5-test.exe
#
# Dependency resolution only (no compile):
#   cd opencv5-test && vcpkg install --triplet x64-windows --dry-run
cmake_minimum_required(VERSION 3.10)
project(opencv5-test LANGUAGES CXX)

# No COMPONENTS list on purpose: this validates that OpenCVConfig.cmake of the
# opencv5 overlay resolves at all and exports OpenCV_LIBS for every module it built.
find_package(OpenCV CONFIG REQUIRED)

add_executable(opencv5-test main.cpp)
target_link_libraries(opencv5-test PRIVATE ${OpenCV_LIBS})
