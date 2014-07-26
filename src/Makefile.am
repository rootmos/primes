bin_PROGRAMS = primes

AM_CXXFLAGS = $(INTI_CFLAGS) $(PTHREAD_CFLAGS) $(BOOST_CPPFLAGS)

primes_SOURCES = primes.cpp blocking_queue.hpp debug.hpp debug.cpp chunk_queue.hpp constants.cpp constants.hpp
primes_LDADD = $(INTI_LIBS) $(PTHREAD_LIBS) $(BOOST_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LIBS)
