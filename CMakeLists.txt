
cmake_minimum_required(VERSION 3.23)
project(circuit-simulator)

set(ADDITIONAL_WARNINGS "")

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(ADDITIONAL_WARNINGS "-Wall -Wextra -Wpedantic -Werror")

    set(
        ADDITIONAL_WARNINGS
        -Wpedantic
        -Wextra
        -Wshadow
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wcast-align
        -Wunused
        -Woverloaded-virtual
        # -Wconversion
        -Wsign-conversion
        -Wuninitialized
        -Werror
    )

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(
        ADDITIONAL_WARNINGS
        /W4
        /WX
    )
endif()

add_compile_options(${ADDITIONAL_WARNINGS})


add_executable(
    circuit-simulator
    src/main.cpp
    )

target_compile_features(
    circuit-simulator
    PRIVATE
    cxx_std_20
    )

file(
    COPY
    data
    DESTINATION
    .
    )


enable_testing()

add_subdirectory(test)


