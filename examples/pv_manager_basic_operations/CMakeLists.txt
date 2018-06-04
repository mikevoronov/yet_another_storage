cmake_minimum_required(VERSION 3.8)

project(pv_manager_basic_operations LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(-std=gnu++0x)
endif()

include_directories("${PROJECT_SOURCE_DIR}/include")

find_package(Threads REQUIRED)
add_library(stdc++fs UNKNOWN IMPORTED)
set_property(TARGET stdc++fs PROPERTY IMPORTED_LOCATION "/usr/lib/gcc/x86_64-linux-gnu/7/libstdc++fs.a")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_FORCE_INLINES")


file(GLOB PV_TEST1 *.cpp)
add_executable(pv_manager_basic_operations ${PV_TEST1})
TARGET_LINK_LIBRARIES(
			pv_manager_basic_operations
			stdc++fs)
