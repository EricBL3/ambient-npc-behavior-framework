cmake_minimum_required(VERSION 3.16)
project(AmbientCoreFramework LANGUAGES CXX)

# Automatically find all source files


file(GLOB_RECURSE INTERNAL_SOURCES "src/*.cpp")
file(GLOB_RECURSE INTERNAL_HEADERS "src/*.h")

# Remove external interface directory from internal sources
file(GLOB_RECURSE INTERFACE_SOURCES "src/integration/external_interfaces/*.cpp")
list(REMOVE_ITEM INTERNAL_SOURCES ${INTERFACE_SOURCES})


file(GLOB_RECURSE INTERFACE_HEADERS "include/*.h")

# Static library for testing internal code
add_library(AmbientCoreFramework_Static STATIC
    ${INTERNAL_SOURCES}
    ${INTERNAL_HEADERS}
)

# DLL Library
add_library(AmbientCoreFramework SHARED
    ${INTERNAL_SOURCES}
    ${INTERNAL_HEADERS}
    ${INTERFACE_SOURCES}
    ${INTERFACE_HEADERS}
)


# Properties for both libraries
set_target_properties(AmbientCoreFramework AmbientCoreFramework_Static PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
)

# Include directories for both libraries
target_include_directories(AmbientCoreFramework PUBLIC
    include # Public headers for the engine
    src # Internal headers for interface implementations
)
target_include_directories(AmbientCoreFramework_Static PUBLIC
    src # Internal headers only for testing
)

# ============== DEPENDENCIES SETUP ==============
include(FetchContent)

# Fetch nlohmann_json
FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.12.0
)

# Fetch Tracy Profiler
FetchContent_Declare(
        tracy
        GIT_REPOSITORY https://github.com/wolfpld/tracy.git
        GIT_TAG v0.12.2
        GIT_SHALLOW TRUE
)

# Configure Tracy options BEFORE making it available
set(TRACY_ENABLE ON CACHE BOOL "Enable Tracy profiler" FORCE)
set(TRACY_ON_DEMAND ON CACHE BOOL "On-demand profiling" FORCE)

FetchContent_MakeAvailable(nlohmann_json tracy)

# Link libraries
target_link_libraries(AmbientCoreFramework PRIVATE
        nlohmann_json::nlohmann_json
        TracyClient
)

target_link_libraries(AmbientCoreFramework_Static PRIVATE
        nlohmann_json::nlohmann_json
        TracyClient
)

# Google Test setup
find_package(GTest CONFIG REQUIRED)
enable_testing()

# Automatically find all test files
file(GLOB_RECURSE TEST_SOURCES "tests/*.cpp")

add_executable(AmbientCoreTests ${TEST_SOURCES})

target_link_libraries(AmbientCoreTests
    PRIVATE
    AmbientCoreFramework_Static
    GTest::gmock_main
    nlohmann_json::nlohmann_json
)

target_include_directories(AmbientCoreTests PRIVATE
        src  # Tests need access to internal headers
)

include (GoogleTest)
gtest_discover_tests(AmbientCoreTests)