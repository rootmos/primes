cmake_minimum_required (VERSION 2.8)

project (primes)

# We'll work with C++11
add_definitions (-std=c++11)

# Require multithreading
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package (Threads REQUIRED)
if (CMAKE_USE_PTHREADS_INIT)
    list (APPEND CMAKE_CXX_FLAGS "-pthread")
endif ()

# We include the boost libraries
find_package (Boost 1.54 COMPONENTS program_options REQUIRED)
include_directories ("${Boost_INCLUDE_DIR}")


# We include the cppformat headers and libraries
include_directories ("${PROJECT_SOURCE_DIR}/cppformat")
link_directories ("${PROJECT_SOURCE_DIR}/cppformat")


# Our options
if (CMAKE_BUILD_TYPE MATCHES DEBUG)
    option (NTIMING "Disable timings" OFF)
    option (NTRACES "Disable traces" OFF)
else()
    option (NTIMING "Disable timings" ON)
    option (NTRACES "Disable traces" ON)
endif()

set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -pg")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast")


add_subdirectory (src)
