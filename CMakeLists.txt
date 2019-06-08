cmake_minimum_required(VERSION 3.14)
project(QuaRTS
    VERSION 0.1.0
    DESCRIPTION "Runtime strategy game and all the related tools"
)

if (EXISTS ${CMAKE_CURRENT_BINARY_DIR}/conan_paths.cmake)
    message(STATUS "Found conan_paths.cmake, using for finding dependencies.")
    include(${CMAKE_CURRENT_BINARY_DIR}/conan_paths.cmake)
endif()

if (NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17 CACHE STRING "")
endif()

if(CMAKE_CXX_STANDARD LESS 17)
    message(FATAL_ERROR "Incompatible C++ standard ${CMAKE_CXX_STANDARD}.")
endif()

add_library(QuaRTS.Base OBJECT)
target_sources(QuaRTS.Base PRIVATE src/GameRules.cpp)

add_executable(QuaRTS)
target_sources(QuaRTS PRIVATE src/QuaRTS.cpp)
target_link_libraries(QuaRTS PRIVATE QuaRTS.Base)

option(USE_UNIT_TESTS "Build and run unit tests during the build." ON)
if (USE_UNIT_TESTS)
    find_package(Catch2 REQUIRED)
    add_executable(QuaRTS.UT)
    target_sources(QuaRTS.UT 
        PRIVATE
            src/GameRules.Test.cpp
            src/Main.Test.cpp
    )
    target_link_libraries(QuaRTS.UT PRIVATE QuaRTS.Base Catch2::Catch2)
    add_custom_command(
        TARGET QuaRTS.UT POST_BUILD
        COMMAND QuaRTS.UT
    )
endif()