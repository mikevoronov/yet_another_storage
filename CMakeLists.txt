cmake_minimum_required(VERSION 3.8)

project(YAS LANGUAGES CXX)

enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(WERROR "Add -Werror flag to build (turns warnings into errors)" OFF)

# configure optimization
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(OPTIMIZATION_FLAGS "-O0 -DDEBUG")
    message("-- Configuring debug build")
else()
    set(OPTIMIZATION_FLAGS "-O3 -DNDEBUG")
    message("-- Configuring release build")
endif()

set(DESIRED_WARNINGS "-Wall -Wextra -Wconversion -Wunreachable-code -Wuninitialized -Wold-style-cast -Wno-error=unused-variable -Wshadow -Wfloat-equal -Weffc++")
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(DESIRED_WARNINGS "${DESIRED_WARNINGS} -Wmost")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OPTIMIZATION_FLAGS} ${DESIRED_WARNINGS}")

if (WERROR)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
endif()

include_directories("${PROJECT_SOURCE_DIR}/include")
add_subdirectory(test)
add_subdirectory(examples/pv_manager_basic_operations)

# libbenchmark.a supports threads and therefore needs pthread support
#find_package(Threads REQUIRED)
#file(GLOB BENCH_SOURCES bench/*.cpp)
#add_executable(bench-tests ${BENCH_SOURCES})