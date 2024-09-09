
cmake_minimum_required(VERSION 3.23)
project(circuit-simulator)

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
