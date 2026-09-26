cmake_minimum_required(VERSION 3.10.0)
project(testcap VERSION 0.1.0 LANGUAGES C CXX)

# Consume the vcpkg opencv4 overlay port only when explicitly requested, so the
# project still configures on machines where opencv4 is not installed yet.
option(TESTCAP_WITH_OPENCV "Build against the vcpkg opencv4 overlay port" OFF)

add_executable(testcap main.cpp)

if(TESTCAP_WITH_OPENCV)
    find_package(OpenCV CONFIG REQUIRED COMPONENTS core)
    target_link_libraries(testcap PRIVATE ${OpenCV_LIBS})
    target_compile_definitions(testcap PRIVATE TESTCAP_WITH_OPENCV)
endif()

