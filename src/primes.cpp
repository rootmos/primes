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

// sqrt (32452843) = 5696 > 5695 = 3+2*2846
#define buffer_length 2847

// 751th prime = 5701, need only 749 since we skip the two
#define factor_length 749

uint factors[factor_length];

#define chunk_length 1000000

struct output_chunk
{
    char *buffer;
    int length;
};


static chunk_queue queue;
static blocking_queue<output_chunk> output_queue;


// TODO: estimate this at program start
size_t max_digits = 100;
uint find_number_of_primes = 2000000;
#define OUTPUT_CHUNK_LENGTH 2000000

static std::atomic_bool running(true);



// The primitive "non-offset" sieve

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
    time_function ();
    uint i = 0;

    while (i < buffer_length)
    {
        fill (odds, i);

        while (odds[++i])
        {
        }

        // trace (("Found prime %d at %d.", 3+2*i, i));
    }
}



// The thread for splitting chunks into newline separated output chunks. We
// also count the number of primes here.

void split_chunks_into_output_chunks ()
{
    time_function ();
    uint number_of_primes = 1;
    bool splitting = true;
    output_chunk oc;
    while (splitting)
    {
        chunk c = queue.pop ();

        oc.buffer = new char[OUTPUT_CHUNK_LENGTH+1];
        char* output_itr = oc.buffer;

        bool* sieve_itr = c.data;
        bool* end = c.data + c.length;
        while ( sieve_itr < end )
        {
            if (!(*sieve_itr))
            {
                number_of_primes++;
                using namespace boost::spirit;
                using boost::spirit::karma::generate;

                generate(output_itr, uint_, 2*(sieve_itr - c.data) + c.offset);
                *output_itr = '\n';
                output_itr++;
            
                if (number_of_primes >= find_number_of_primes)
                {
                    splitting = false;
                    break;
                }
            }

            sieve_itr++;
        }

        oc.length = output_itr-oc.buffer;
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

    while (running || !output_queue.empty ())
    {
        output_chunk oc = output_queue.pop ();

        fwrite (oc.buffer, oc.length, sizeof (char), file);
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

    for (uint i = 0; i < factor_length; i++)
    {
        fill_offset (chunk, factors[i], from, length);
    }

    queue.push (chunk, from, length);

    std::this_thread::yield ();

    if ( to - from  > 2*chunk_length )
        offset_sieve (from + 2*length, to);
}



// The main main function

int main()
{
    time_function ();

    // Start our two auxillary threads

    std::thread split_thread(split_chunks_into_output_chunks);
    std::thread output_thread(output_worker);

    // Let's find the first factors we need

    bool odds[buffer_length] = { false };
    sieve (odds);

    // Output the first factors we've found
    queue.push (odds, 3, buffer_length);

    // Generate the factors we are going to need

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
        itr++;
    }


    // Start the workers and assign them regions to sieve 

    std::thread* workers[THREADS];

    uint from = 3+buffer_length*2;
    uint to = 32452845;
    uint end;
    uint assignment_length = (to - from)/THREADS + 1;

    if (assignment_length % 2 == 1)
        assignment_length++;

    for (int j = 0; j < THREADS; j++)
    {
        if (j+1 == THREADS)
            end = to;
        else
            end = from + assignment_length;

        workers[j] = new std::thread (offset_sieve, from, end);
        from += assignment_length;
    }

    // Wait for the workers to finish sieving
    
    for (int j = 0; j < THREADS; j++)
        workers[j]->join();

    // Wait for the auxillary threads

    split_thread.join();
    running = false;
    output_thread.join ();

    // Success!

    return 0;
}


