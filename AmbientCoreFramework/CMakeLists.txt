cmake_minimum_required(VERSION 3.16)
project(AmbientCoreFramework LANGUAGES CXX)

# Automatically find all source files
file(GLOB_RECURSE FRAMEWORK_SOURCES "src/*.cpp")
file(GLOB_RECURSE FRAMEWORK_HEADERS "src/*.h")

# DLL Library
add_library(AmbientCoreFramework SHARED ${FRAMEWORK_SOURCES} ${FRAMEWORK_HEADERS})
# Static library for testing
add_library(AmbientCoreFramework_Static STATIC ${FRAMEWORK_SOURCES} ${FRAMEWORK_HEADERS})
# Properties for both libraries
set_target_properties(AmbientCoreFramework AmbientCoreFramework_Static PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
)

# Include directories for both libraries
target_include_directories(AmbientCoreFramework PUBLIC src)
target_include_directories(AmbientCoreFramework_Static PUBLIC src)

# Google Test setup
find_package(GTest CONFIG REQUIRED)
enable_testing()

# Automatically find all test files
file(GLOB_RECURSE TEST_SOURCES "tests/*.cpp")

add_executable(AmbientCoreTests ${TEST_SOURCES})

target_link_libraries(AmbientCoreTests
    PRIVATE
    AmbientCoreFramework_Static
    GTest::gtest_main
)

include (GoogleTest)
gtest_discover_tests(AmbientCoreTests)