#include "debug.hpp"
#include "config.h"
#include "constants.hpp"

#ifndef NTESTING

uint* cheat = new uint[number_of_primes];

void read_cheat ()
{
    FILE* file = fopen ("cheat", "r");

    assert (file != nullptr);

    for (uint i = 0; i < number_of_primes+1; i++)
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


#ifndef NTIMING

std::mutex time_block_class::cerr_lock;

time_block_class::time_block_class (const char* m) : message (m)
{
    clock_gettime (clock, &start);
}

time_block_class::~time_block_class ()
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


