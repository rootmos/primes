bin_PROGRAMS = primes

AM_CXXFLAGS = $(INTI_CFLAGS) $(PTHREAD_CFLAGS) $(BOOST_CPPFLAGS) $(CPPFORMAT_INCL)

primes_SOURCES = primes.cpp blocking_queue.hpp debug.hpp debug.cpp chunk_queue.hpp constants.cpp constants.hpp chunk.hpp chunk.cpp
primes_LDADD = $(INTI_LIBS) $(PTHREAD_LIBS) $(BOOST_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LIBS) $(CPPFORMAT_LIBS)
