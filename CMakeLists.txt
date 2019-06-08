cmake_minimum_required(VERSION 3.14)
project(QuaRTS
    VERSION 0.1.0
    DESCRIPTION "Runtime strategy game and all the related tools"
)

add_executable(QuaRTS)
target_sources(QuaRTS PRIVATE src/QuaRTS.cpp)
target_compile_features(QuaRTS PRIVATE cxx_std_17)
