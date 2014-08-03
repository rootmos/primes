
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/config.h)
include_directories ("${CMAKE_CURRENT_BINARY_DIR}")

add_executable (primes primes.cpp chunk.cpp constants.cpp debug.cpp)
add_dependencies (primes format)
target_link_libraries (primes ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES} "libformat.a") 
