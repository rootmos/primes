#include <thread>
#include <cassert>
#include <atomic>
#include <boost/spirit/include/karma.hpp>

#include "chunk_queue.hpp"
#include "blocking_queue.hpp"
#include "debug.hpp"
#include "constants.hpp"

#include "config.h"


using uint = unsigned int;

uint* factors;

typedef offset_chunk<bool> split_chunk;
typedef offset_chunk<char> output_chunk;

static blocking_queue<split_chunk> split_queue;
static offset_chunk_queue<char> output_queue;


// The primitive "non-offset" sieve

inline void fill (bool* odds, uint i, uint length)
{
    uint n = 3 + 2*i;

    for (uint j = i + n; j < length; j += n)
    {
        odds[j] = true;
    }
}

void sieve (bool* odds, uint length)
{
    time_function ();
    uint i = 0;

    trace (("Initial sieve: from=3 to=%d.", 1+length*2));

    while (i < length)
    {
        fill (odds, i, length);

        while (odds[++i])
        {
        }

        //trace (("Found prime %d at %d.", 3+2*i, i));
    }
}



// The thread for splitting chunks into newline separated output chunks. We
// also count the number of primes here.

std::atomic_uint current_number_of_primes(1);

void split_chunks_into_output_chunks ()
{
    time_function ();
    output_chunk oc;
    split_chunk c;
    while (split_queue.pop (c))
    {
        oc.data = new char[output_chunk_length+1];
        char* output_itr = oc.data;

        bool* sieve_itr = c.data;
        bool* end = c.data + c.length;
        while ( sieve_itr < end )
        {
            if (!(*sieve_itr))
            {
                using namespace boost::spirit;
                using boost::spirit::karma::generate;

                generate(output_itr, uint_, 2*(sieve_itr - c.data) + c.offset);
                *output_itr = '\n';
                output_itr++;

                if (++current_number_of_primes >= number_of_primes)
                {
                    split_queue.stop ();
                    break;
                }
            }

            sieve_itr++;
        }

        oc.length = output_itr-oc.data;
        oc.offset = c.offset;
        oc.next_offset = c.offset + 2*c.length;
        trace (("Pushing chunk with offset %d for output. Next chunk to write is %d", c.offset, oc.next_offset));
        output_queue.push (oc);
        //delete [] c.data;
    }

}




// The output worker thead, which basically just pops the output queue and
// writes the chunks into the file

void output_worker ()
{
    time_function ();
    FILE* file = fopen ("output", "w");

    assert (file != nullptr);

    fwrite ("2\n", 2, sizeof (char), file);

    output_chunk oc;

    while (output_queue.pop (oc))
    {
        fwrite (oc.data, oc.length, sizeof (char), file);

        trace (("Wrote chunk with offset %d.", oc.offset));
        //delete [] oc.buffer;
    }

    fclose (file);
}



// The two methods used in the offset sieve worker threads

inline void fill_offset (bool* chunk, uint p, uint offset, uint length)
{
    assert (offset % 2 == 1);
    uint i;
    if ( offset % p == 0)
        i = 0;
    else
    {
        i = offset / p + 1;

        if ( i % 2 == 0 )
            i += 1;

        i = i*p - offset;

        i /= 2;
    }

    //trace (("Filling i=%d for p=%d with offset=%d.", i, p, offset));

    while (i < length)
    {
        chunk[i] = true;
        i += p;
    }
}


void offset_sieve (uint from, uint to)
{
    time_function ();
    bool* chunk = new bool[chunk_length];
    uint length = to - from;
    length = (length > 2*chunk_length ? chunk_length : length/2);

    trace (("Sieving from %d to %d.", from, from+2*length));

    for (uint i = 0; i < number_of_factors; i++)
    {
        fill_offset (chunk, factors[i], from, length);
    }

    split_queue.push (split_chunk (chunk, from, length));

    std::this_thread::yield ();

    if ( to - from  > 2*chunk_length )
        offset_sieve (from + 2*length, to);
}



// The main main function

int main(int argc, char* argv[])
{
    time_function ();

    // First we parse our options

    if (!parse_options (argc, argv))
        return 1;

    // Start our two auxillary threads

    std::thread** splitters = new std::thread*[splitting_threads];

    uint i;
    for (i = 0; i < splitting_threads; i++)
        splitters[i] = new std::thread (split_chunks_into_output_chunks);

    std::thread output_thread(output_worker);

    // Let's find the first factors we need

    bool* odds = new bool[number_of_odds_to_find_factors];
    for (i = 0; i < number_of_odds_to_find_factors; i++)
        odds [i] = false;

    sieve (odds, number_of_odds_to_find_factors);

    // Output the first factors we've found
    split_queue.push (split_chunk (odds, 3, number_of_odds_to_find_factors));

    // Generate the factors we are going to need

    factors = new uint[number_of_factors];
    number_of_factors = 0;
    for (i = 0; i < number_of_odds_to_find_factors; i++)
    {
        if (!odds[i])
        {
            factors[number_of_factors++] = 3 + 2*i;
            //trace (("Factor %d=%d.", number_of_factors, factors[number_of_factors-1]));
        }
    }

    // Start the workers and assign them regions to sieve

    std::thread** workers = new std::thread*[sieving_threads];

    uint from = 3+number_of_odds_to_find_factors*2;
    uint to = nth_prime;
    uint end;
    uint assignment_length = (to - from)/sieving_threads + 1;

    if (assignment_length % 2 == 1)
        assignment_length++;

    for (uint j = 0; j < sieving_threads; j++)
    {
        if (j+1 == sieving_threads)
            end = to;
        else
            end = from + assignment_length;

        workers[j] = new std::thread (offset_sieve, from, end);
        from += assignment_length;
    }

    // Wait for the workers to finish sieving

    for (i = 0; i < sieving_threads; i++)
        workers[i]->join();

    trace (("All sieving threads done."));

    // Wait for the auxillary threads

    for (i = 0; i < splitting_threads; i++)
        splitters[i]->join();

    trace (("All splitting threads done."));

    output_queue.stop ();

    output_thread.join ();

    // Success!

    return 0;
}


