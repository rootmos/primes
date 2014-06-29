#include <iostream>
#include <thread>
#include <cassert>
#include <cmath>
#include <chrono>

#include <cstring>

// For traces and cheat
#include <stdio.h>

#include "config.h"

using uint = unsigned int;

#ifndef NTRACES
#define trace(vars) printf vars
#else
#define trace(vars)
#endif

#ifndef NTESTING
// Read the cheat-file for comparison
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

#else

#define test (i, prime) 

#endif


// sqrt (32452843) = 5696 > 5695 = 3+2*2846
#define buffer_length 2847

// 751th prime = 5701, need only 749 since we skip the two
#define factor_length 749

uint factors[factor_length];

#define chunk_length 10000

inline void fill (bool* odds, uint i)
{
    uint n = 3 + 2*i;

    for (uint j = i + n; j < buffer_length; j += n)
    {
        odds[j] = true;
    }
}

void sieve (bool* odds)
{
    uint i = 0;

    while (i < buffer_length)
    {
        fill (odds, i);
        
        while (odds[++i])
        {
        }

        trace (("Found prime %d at %d.\n", 3+2*i, i)); 
    }
}

inline void fill_offset (bool* chunk, uint p, uint offset, uint length)
{
    assert (offset % 2 == 1);
    uint i;
    if ( offset % p == 0)
        i = 0;
    else
    {
        float k = float(offset) / float(p);
        i = offset / p + 1; //ceil (k);

        if ( i % 2 == 0 )
            i += 1;

        i = i*p - offset;

        i /= 2;
    }
    
    trace (("Filling i=%d for p=%d with offset=%d.\n", i, p, offset));
    

    while (i < length)
    {
        chunk[i] = true;
        i += p; 
    }
}

void output (bool* odds, uint offset, uint length)
{
    for (uint i = 0; i < length; i++)
    {
        if (!odds[i])
           std::cout << offset + 2*i << std::endl;
    }
}


void offset_sieve (bool* chunk, uint from, uint to)
{
    uint length = to - from;
    length = (length > 2*chunk_length ? chunk_length : length/2);

    trace (("Sieving from %d to %d.\n", from, from+2*length));

    for (uint i = 0; i < factor_length; i++)
    {
        fill_offset (chunk, factors[i], from, length);
    }

    output (chunk, from, length); 

    if ( to - from  > 2*chunk_length )
    {
        for (uint j = 0; j < chunk_length; j++ )
            chunk[j] = false;
        //memset (chunk, 0, chunk_length);
        offset_sieve (chunk, from + 2*length, to);
    }
}

void offset_sieve (uint from, uint to)
{
    bool chunk[chunk_length] = { false };

    offset_sieve (chunk, from, to);
}

// The main main function

int main()
{
    std::cout << 2 << std::endl;

    bool odds[buffer_length] = { false };
    sieve (odds);

    uint i = 0;
    bool* itr = odds;
    for (i = 0; i < factor_length; i++)
    {
        while(*itr)
        {
            itr++;
            if (itr >= odds + buffer_length)
                break;
        }
        factors[i] = 3 + 2*(itr-odds);
        std::cout << factors[i] << std::endl;
        itr++;
    }

    offset_sieve ( factors[i-1]+2, 32452845 );
    
    return 0;
}


