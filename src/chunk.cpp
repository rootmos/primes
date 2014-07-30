#include "chunk.hpp"
#include "odds.hpp"
#include "constants.hpp"
#include <cassert>
#include <algorithm>
#include <boost/spirit/include/karma.hpp>

#include "debug.hpp"
#include "wheel.hpp"

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
    uint i;
    if ( from % p == 0)
        i = 0;
    else
    {
        i = from / p + 1;

        if ( i % 2 == 0 )
            i += 1;

        i = i*p - from;

        i /= 2;
    }

    while (i < odds_length)
    {
        if (!odds[i])
        {
            odds[i] = true;
            primes -= 1;
        }
        i += p;
    }
}

// Function to sieve the chunk

void
chunk::sieve (uint* factors, uint factors_length)
{
    //time_function ();

    impl->primes = impl->odds_length;

    for (uint i = 0; i < factors_length; ++i)
    {
        impl->fill_offset (factors[i]);
    }
    
    trace (("Sieving from %d to %d. Found %u primes.", impl->from, impl->to, impl->primes));
}

// Function to prepare the chunk for output

void
chunk::chunk_impl::prepare_for_output ()
{
    //time_function ();
    
    char* output_itr = output = new char[output_chunk_length];//primes * (number_of_digits+1)];

    auto odds_itr = the_wheel.iterate_from (from);

    uint i = odds_itr.position ();
    while (i < odds_length)
    {
        if (odds[i])
        {
            i = odds_itr.next ();
            continue;
        }

        using namespace boost::spirit;
        using boost::spirit::karma::generate;

        uint prime = 2*i + from;

        generate(output_itr, uint_, prime);

        *(output_itr++) = '\n';

        i = odds_itr.next ();
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

    primes = m;
    odds_length = i+1;
}

