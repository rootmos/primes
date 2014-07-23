#include "debug.hpp"



#ifndef NTESTING

uint cheat[TOTAL_PRIMES] = { 0 };

void read_cheat ()
{
    FILE* file = fopen ("cheat", "r");

    assert (file != nullptr);

    for (uint i = 0; i < TOTAL_PRIMES+1; i++)
        assert ( fscanf (file, "%d", &cheat[i]) == 1 );

    fclose (file);
}

bool test (uint i, uint prime)
{
    if ( cheat[i] == 0 )
        read_cheat ();

    return prime == cheat[i];
}

#endif


#ifdef TIMING

std::mutex time_block_class::cerr_lock;

inline time_block_class::time_block_class (const char* m) : message (m)
{
    clock_gettime (clock, &start);
}

inline time_block_class::~time_block_class ()
{
    timespec end;
    clock_gettime (clock, &end);
    long ns_elapsed = (end.tv_sec - start.tv_sec)*1000000000
        + (end.tv_nsec - start.tv_nsec);
    cerr_lock.lock ();
    std::cerr << message << ": " << ns_elapsed/res << std::endl;
    cerr_lock.unlock ();
};

#endif


