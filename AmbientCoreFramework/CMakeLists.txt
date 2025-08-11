cmake_minimum_required(VERSION 3.16)
project(AmbientCoreFramework LANGUAGES CXX)

add_library(AmbientCoreFramework SHARED src/main.cpp)

set_target_properties(AmbientCoreFramework PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
)