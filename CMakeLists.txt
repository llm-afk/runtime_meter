cmake_minimum_required(VERSION 3.10)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(runtime_meter C)
endif()

add_library(runtime_meter STATIC
    ${CMAKE_CURRENT_LIST_DIR}/src/runtime_meter.c)
add_library(runtime_meter::runtime_meter ALIAS runtime_meter)

target_include_directories(runtime_meter
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}/inc
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/port)
target_compile_features(runtime_meter PUBLIC c_std_99)

if(RUNTIME_METER_PORT_SOURCE)
    target_sources(runtime_meter PRIVATE ${RUNTIME_METER_PORT_SOURCE})
endif()
if(RUNTIME_METER_PORT_LIBRARIES)
    target_link_libraries(runtime_meter PRIVATE ${RUNTIME_METER_PORT_LIBRARIES})
endif()

option(RUNTIME_METER_BUILD_TESTS "Build runtime_meter unit tests" OFF)
if(RUNTIME_METER_BUILD_TESTS)
    enable_testing()
    add_executable(runtime_meter_tests
        ${CMAKE_CURRENT_LIST_DIR}/tests/runtime_meter_tests.c
        ${CMAKE_CURRENT_LIST_DIR}/src/runtime_meter.c)
    target_include_directories(runtime_meter_tests PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/inc
        ${CMAKE_CURRENT_LIST_DIR}/port)
    target_compile_features(runtime_meter_tests PRIVATE c_std_99)
    add_test(NAME runtime_meter_tests COMMAND runtime_meter_tests)
endif()
