cmake_minimum_required(VERSION 3.10)

project(middleware LANGUAGES C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

option(ENABLE_TESTS "Enable building and running unit tests" ON)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_C_FLAGS_DEBUG "-g -O0")
set(CMAKE_C_FLAGS_RELEASE "-O2")

file(GLOB_RECURSE SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

list(FILTER SOURCES EXCLUDE REGEX ".*_linux\\.c$|.*_windows\\.c$")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(middleware_app ${SOURCES})

find_package(Threads REQUIRED)

target_link_libraries(middleware_app PRIVATE Threads::Threads)

if(ENABLE_TESTS)
    enable_testing()

    file(GLOB_RECURSE TEST_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/test/*_test.c"
    )

    file(GLOB_RECURSE LIB_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    )
    list(REMOVE_ITEM LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.c")
    
    list(FILTER LIB_SOURCES EXCLUDE REGEX ".*_linux\\.c$|.*_windows\\.c$")

    foreach(test_source ${TEST_SOURCES})
        get_filename_component(test_name ${test_source} NAME_WE)
        add_executable(${test_name} ${test_source} ${LIB_SOURCES})
        target_include_directories(${test_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
        target_compile_options(${test_name} PRIVATE
            -Wall
            -Wextra
            -Werror
        )
        target_link_libraries(${test_name} PRIVATE Threads::Threads)
        add_test(NAME ${test_name} COMMAND ${test_name})
        set_target_properties(${test_name} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test"
        )
    endforeach()

    add_custom_target(run_tests ALL
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        DEPENDS ${TEST_SOURCES}
        COMMENT "Running unit tests..."
    )
    
    message(STATUS "Testing enabled")
else()
    message(STATUS "Testing disabled (use -DENABLE_TESTS=ON to enable)")
endif()

target_compile_options(middleware_app PRIVATE
    -Wall
    -Wextra
    -Werror
)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "Source files: ${SOURCES}")
message(STATUS "Include directory: ${CMAKE_CURRENT_SOURCE_DIR}/include")
message(STATUS "Enable tests: ${ENABLE_TESTS}")
