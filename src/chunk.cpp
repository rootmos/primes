#include "chunk.hpp"
#include "odds.hpp"
#include "constants.hpp"
#include <cassert>
#include <algorithm>

#include <string.h>
#include <format.h>

#include "debug.hpp"

using uint = unsigned int;



// Our construtctors

chunk::chunk_impl::chunk_impl(uint f, uint t) :
    from (odds::upper (f)), to ( odds::lower (t)),
    primes (0),
    odds_length (odds::number_of_odds_between_odds (from, to)),
    odds (new bool[odds_length]),
    output (nullptr),
    output_length (0)
{
}

chunk::chunk (uint f, uint t) :
    impl (new chunk::chunk_impl (f, t))
{ }


chunk::chunk_impl::chunk_impl (uint f, uint t, bool* o, uint o_l) :
    from (odds::upper (f)), to ( odds::lower (t)),
    primes (0),
    odds_length (o_l),
    odds (o),
    output (nullptr),
    output_length (0)
{}

chunk::chunk (uint f, uint t, bool* odds, uint odds_length) :
    impl (new chunk::chunk_impl (f, t, odds, odds_length))
{ }

chunk::chunk () :
    impl ()
{ }


// Our implementation's destructor

chunk::chunk_impl::~chunk_impl ()
{
    delete [] odds;
    delete [] output;
}


// The function for filling the internal odds with multiples of a prime

inline void
chunk::chunk_impl::fill_offset (uint p)
{
    assert ( odds::is_odd (from) );

    uint i = 0; // We assume that we start filling from the start of the buffer.

    // If we already are at an odd multiple of p then don't change the start.
    if ( from % p != 0 )
    {
        // Otherwise we find the next point in the grid of odd multiples of p

        i = from / p + 1; // Fast version of ceil (from/p), since: from/p != int

        // Then if we ended up at an even multiple go to the next multiple
        if ( odds::is_even (i) )
            i += 1;

        // Lastly we determine the distance between our start and the chosen
        // multiple of p
        i = odds::distance_between_odds (from, i*p);
    }


    while (i < odds_length)
    {
        bool& odd = odds[i];
        i += p;

        // Since we count the primes in the sieve; we check if we've visited
        // the odd before
        if (!odd)
        {
            odd = true;
            --primes; // Do the counting
        }
    }
}

// Function to sieve the chunk

void
chunk::sieve (uint* factors, uint factors_length)
{
    // We count at the same time we are sieving, so a priori we have the same
    // number of primes as our buffer is long
    impl->primes = impl->odds_length;

    for (uint i = 0; i < factors_length; ++i)
    {
        impl->fill_offset (factors[i]);
    }

    trace (("Sieving from %d to %d. Found %u primes.",
            impl->from, impl->to, impl->primes));
}


// Function to prepare the chunk for output

void
chunk::chunk_impl::prepare_for_output ()
{
    char* output_itr = output = new char[output_chunk_length];

    for (uint i = 0; i < odds_length; ++i)
    {
        if (odds[i])
            continue;

        uint prime = 2*i + from;

        auto string = fmt::FormatInt(prime);
        size_t length = string.size ();

        memcpy (output_itr, string.c_str(), length);

        output_itr += length;

        *(output_itr++) = '\n';
    }

    output_length = output_itr - output;
}


// Function for extracting the output string

void
chunk::c_str (const char*& buffer, size_t& length) const
{
    buffer = impl->output;
    length = impl->output_length;
}


// Function for resizing the chunk

void
chunk::chunk_impl::resize (uint n)
{
    // We determine how long into the sieve we need to go before encountering n
    // primes
    uint m = 0;
    uint i = 0;

    for (; i < odds_length; ++i)
    {
        if (odds[i])
            continue;

        if (++m == n)
            break;
    }

    assert (m == n && "Didn't find enough primes in the sieve!");


    // Then set the length of the buffer.
    primes = m;
    odds_length = i+1; // + 1 since we want to include the prime we stoped at
}

