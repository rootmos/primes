#include <thread>
#include <cassert>
#include <cmath>
#include <algorithm>
#include <functional>

#include <boost/scoped_array.hpp>

#include "chunk_queue.hpp"
#include "blocking_queue.hpp"
#include "debug.hpp"
#include "constants.hpp"
#include "chunk.hpp"

#include "config.h"


using uint = unsigned int;

std::vector<uint> factors;

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



blocking_queue<chunk> sieved_chunks;
blocking_queue<chunk, std::priority_queue<chunk> > splitted_chunks;

void sieving_thread (std::unique_ptr<std::vector<chunk> > chunks)
{
    time_function ();

    for (std::vector<chunk>::iterator itr = chunks->begin ();
         itr != chunks->end ();
         itr++)
    {
        chunk& c = *itr;
        c.sieve (factors);

        // Force a count. TODO: can we move this to a more efficient place?
        c.size ();

        sieved_chunks.push (c);
    }
}

void splitting_thread ()
{
    time_function ();

    chunk c;
    while (sieved_chunks.pop (c))
    {
        c.prepare_for_output ();
        splitted_chunks.push (c);
    }
}


class pop_next_chunk
{
    uint next = 3;

public:

    bool operator() (const chunk& c)
    {
        if (c.from () == next)
        {
            next = c.to () + 2;
            trace (("Poped chunk with offset %d. Next will be %d.", c.from (), next));
            return true;
        }
        else
            return false;
    };
};


void output_worker ()
{
    time_function ();
    FILE* file = fopen ("output", "w");

    assert (file != nullptr);

    fwrite ("2\n", 2, sizeof (char), file);

    chunk c;
    const char* buffer;
    size_t length;

    std::function<bool(const chunk&)> predicate = pop_next_chunk();

    while (splitted_chunks.pop (c, predicate))
    {
        trace (("Writing chunk starting with %d.", c.from ()));
        c.c_str (buffer, length);
        fwrite (buffer, length, sizeof (char), file);
    }

    fclose (file);
}



void prepare_factors ()
{
    time_function ();

    boost::scoped_array<bool> odds(new bool[number_of_odds_to_find_factors]);

    // Sieve the odds

    sieve (odds.get (), number_of_odds_to_find_factors);

    // Get the factors out of the sieve

    factors.reserve (number_of_factors);
    number_of_factors = 0;
    for (uint i = 0; i < number_of_odds_to_find_factors; i++)
    {
        if (!odds[i])
        {
            factors[number_of_factors++] = 3 + 2*i;
        }
    }

}





// The main main function

int main(int argc, char* argv[])
{
    time_function ();

    // First we parse our options

    if (!parse_options (argc, argv))
        return 1;

    // Start our two types of auxillary threads
    
    std::vector<std::thread> splitters;

    for (uint i = 0; i < splitting_threads; i++)
        splitters.push_back(std::thread (splitting_thread));

    std::thread output_thread(output_worker);


    // Generate the factors we are going to need

    prepare_factors ();


    // Let's figure out the sievers' assignments first

    uint start = 3+number_of_odds_to_find_factors*2;
    uint end = nth_prime_below;

    // The assignment vectors will be handed over to the threads together with
    // the responsibility to remove them...
    std::vector<std::unique_ptr<std::vector<chunk> > > assignments;

    for (uint i = 0; i < sieving_threads; i++)
        assignments.push_back
            (std::move(std::unique_ptr<std::vector<chunk> >
             (new std::vector<chunk>)));

    // Spread out the chunks across the threads

    uint siever = 0;
    uint from = start;
    uint to = from + chunk_length;

    do
    {
        assignments[siever++]->push_back (chunk (from, to));
        siever %= assignments.size ();
        from = to + 1;
        to = from + chunk_length;
        if (to > end)
            to = end;
    }
    while (from < end);
    
    // Let's start the sieving threads

    std::vector<std::thread> sievers;

    std::for_each
        (assignments.begin (), assignments.end (),
         [&sievers] (std::unique_ptr<std::vector<chunk> >& assignment)
         { sievers.push_back
            (std::thread (sieving_thread, std::move(assignment))); });


    // TODO: do something with the last chunk!



    // Wait for the workers to finish sieving

    std::for_each
        (sievers.begin (), sievers.end (),
         [] (std::thread& thread)
         { thread.join(); });

    trace (("All sieving threads done."));

    // Wait for the auxillary threads
    
    std::for_each
        (splitters.begin (), splitters.end (),
         [] (std::thread& thread)
         { thread.join(); });

    trace (("All splitting threads done."));

    output_queue.stop ();

    output_thread.join ();

    // Success!

    return 0;
}


