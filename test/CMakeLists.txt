
add_executable(
    sim-tests
    result_test.cpp
    mls-unit-test/test_main.cpp
)

target_compile_features(
    sim-tests
    PRIVATE
    cxx_std_20
    )

add_test(
    COMMAND ./test
)

target_include_directories(
    sim-tests
    PRIVATE
    ../src
)
