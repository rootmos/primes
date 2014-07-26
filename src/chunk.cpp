#include "chunk.hpp"
#include "debug.hpp"
#include "odds.hpp"
#include "constants.hpp"
#include <cassert>
#include <algorithm>
#include <boost/spirit/include/karma.hpp>

using uint = unsigned int;



// Our construtctors

chunk::chunk_impl::chunk_impl(uint f, uint t):
    from (odds::upper (f)), to ( odds::lower (t)),
    primes (0),
    odds (odds::number_of_odds_between_odds (from, to)),
    output ()
{ }

chunk::chunk (uint f, uint t) :
    impl (new chunk::chunk_impl (f, t))
{ }


// The ordering of our chunks

inline bool
chunk::operator<(const chunk& rhs)
{
    return impl->from < rhs.impl->from;
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

    while (i < odds.size ())
    {
        odds[i] = true;
        i += p;
    }
}

// Function to sieve the chunk

void
chunk::sieve (std::vector<bool>& factors)
{
    time_function ();

    trace (("Sieving from %d to %d.", impl->from, impl->to));

    for (uint i = 0; i < factors.size (); i++)
    {
        impl->fill_offset (factors[i]);
    }
}

// Count the number of primes in the sieve

inline uint
chunk::size ()
{
    if (impl->primes == 0)
        impl->do_count ();

    return impl->primes;
}

void
chunk::chunk_impl::do_count ()
{
    assert (odds.size () != 0 && "Need to sieve first!");

    std::for_each (odds.begin (), odds.end (),
                   [this] (bool n) { if (!n) ++primes; });
}


// Function to prepare the chunk for output

void
chunk::chunk_impl::prepare_for_output ()
{
    time_function ();

    // TODO: Perhaps we can afford to to digits*odds.size()...
    output.reserve (output_chunk_length);

    std::string::iterator output_itr = output.begin ();

    for (std::vector<bool>::iterator sieve_itr = odds.begin ();
         sieve_itr != odds.end ();
         ++sieve_itr)
    {
        if (*sieve_itr)
            continue;

        using namespace boost::spirit;
        using boost::spirit::karma::generate;

        generate(output_itr, uint_,
                 2*std::distance(odds.begin (), sieve_itr) + from);

        *(output_itr++) = '\n';
    }
}

