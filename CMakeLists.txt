cmake_minimum_required(VERSION 3.10)
project(network_lib)
message(STATUS "Project name : ${PROJECT_NAME}")

if(NOT UNIX OR APPLE)
    message(FATAL_ERROR "This project supports only Unix-like systems (excluding Apple/macOS).")
endif()

set(CMAKE_CXX_STANDARD 17)

include_directories(headers)

set(CURRENT_DIR ${CMAKE_SOURCE_DIR})
message(STATUS "Current dir:  - ${CURRENT_DIR}")

# Collect all header files (.h, .hpp) from headers directory
file(GLOB HEADER_FILES
    "${CMAKE_SOURCE_DIR}/headers/network/*.h"
    "${CMAKE_SOURCE_DIR}/tests/*.h"
    "${CMAKE_SOURCE_DIR}/status_checker/*.h"
    "${CMAKE_SOURCE_DIR}/server_for_test/*.h"
)

# Collect all source files (.cpp) from specified directories
file(GLOB SOURCE_FILES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/needed_files/*.cpp"
    "${CMAKE_SOURCE_DIR}/status_checker/*.cpp"
    "${CMAKE_SOURCE_DIR}/server_for_test/*.cpp"
)

# Combine header and source files for display or further use
set(ALL_FILES ${HEADER_FILES} ${SOURCE_FILES})

# Print the collected files
message(STATUS "Header files:")
foreach(file ${HEADER_FILES})
    message(STATUS "  ${file}")
endforeach()

message(STATUS "Source files:")
foreach(file ${SOURCE_FILES})
    message(STATUS "  ${file}")
endforeach()

# Build the library with the filtered source files
#add_library(network_lib STATIC ${SOURCE_FILES})
add_library(network_lib SHARED ${SOURCE_FILES})

# Build the test executable with main.cpp
add_executable(test_exec tests/main.cpp)
target_link_libraries(test_exec network_lib)
