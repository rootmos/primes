#ifndef debug_hpp
#define debug_hpp

#include <chrono>
#include <cassert>
#include <mutex>
#include <iostream>

#include <stdio.h>

#include "config.h"

using uint = unsigned int;


// Traces

#ifndef NTRACES
#define trace(vars) printf("%s:\t", __FUNCTION__); printf vars; printf("\n");
#else
#define trace(vars)
#endif


// Cheating functions

#ifndef NTESTING
bool test (uint i, uint prime);
#else
#define test (i, prime)
#endif


#ifndef NTIMING

// Timing functions

class time_block_class
{
    static std::mutex cerr_lock;
    const clockid_t clock = CLOCK_THREAD_CPUTIME_ID;
    const long res = 1000;
    timespec start;
    const char* message;
public:

    time_block_class (const char* m);
    ~time_block_class ();
};

#define time_function() time_block_class t(__FUNCTION__);
#define time_block (m) time_block_class t(m);

#else

#define time_function()
#define time_block (m)

#endif

#endif
