#include <iostream>
#include <thread>
#include <cassert>
#include <cmath>
#include <chrono>

#include "primes.hpp"


#ifndef NDEBUG
#include <stdio.h>
#define trace(vars) printf vars
#else
#define trace(vars)
#endif



// Worker thread's iterator constructor

container::iterator::iterator (container* d):
    i(0),
    stored(0),
    data(d)
{
}


// Worker thread's iterator reset

inline void container::iterator::reset ()
{
    this->i = 0;
}



// Worker thread's iterator next

inline number container::iterator::next ()
{
    i++;

    // We should be able to stay within our thead's cache
    // Perhaps: make this array grow dynamically?
    assert ( i < CHECK_PRIMES );

    if (i < stored)
        return primes[i];

    // We should only need to fetch the next prime
    assert ( i == stored );

    stored++;

    return primes[i] = data->get (i);
}


// The container's get function

inline number container::get (index i)
{
    // We should only try to retrieve clean read-only data
    assert(i < clean);

    return primes[i];
}


// The container's get_iterator function

inline container::iterator container::get_iterator ()
{
    return iterator(this);
}



// The worker thread's constructor

worker_thread::worker_thread (container* d):
    data(d),
    thread(&worker_thread::worker, this)
{
}

// The worker thread's join method

inline void worker_thread::join ()
{
    thread.join();
}


// The worker thread's actuall worker!

void worker_thread::worker ()
{
    container::iterator itr = data->get_iterator();
    number divisor, divisor_limit, remainder;

    while ( data->next_assignment (current, assignment_end) )
    {
        trace (("%d: I was assigned: %d to %d\n",
               this, current, assignment_end));

        // Ensure that we actually start with an odd number
        assert( current % 2 == 1 );

        while ( current <= assignment_end )
        {
            itr.reset();
            divisor_limit = std::sqrt (current);
            remainder = 0;

            // Test digit-sum?

            while ( (divisor = itr.next()) <= divisor_limit )
            {
                remainder = current % divisor;
                if (remainder == 0)
                    break;
            }
            if (remainder != 0)
                data->report_prime (current);

            current += 2; // Of course we skip the even numbers!
        }

        trace (("%d: I finished checking: %d to %d\n",
                this, current, assignment_end));
    }

}


// The container's report_prime method

void container::report_prime (number prime)
{
    trace (("Found prime: %d\n", prime));
    std::lock_guard<std::mutex> lock (report_prime_mutex);

    primes[used] = prime;

    used++;

    assert (used < BUFFER_SIZE); // What should we do when this fails?
}


// The container's next_assignment

bool container::next_assignment (number& first, number& end)
{
    std::unique_lock<std::mutex> lock(assignment_mutex);

    if (are_we_there_yet())
        return false;

    new_clean_primes.wait
        (lock,
         [this]
            { return std::pow(largest_clean(),2) < largest_assigned;} );

    first = largest_assigned + 2;
    assert ( first % 2 == 1 ); // first should be an odd number

    largest_assigned = end = std::pow(largest_clean(),2) / THREADS;

    if (end - first > ASSIGNMENT_MAX) // We should not take to large bites
        end = first + ASSIGNMENT_MAX;
    else if (end % 2 == 1) // and we want to end at an odd number
        end--;

    assert (first < end); // Could the opposite really occur?

    return true;
}


// The container's are_we_there_yet method

inline bool container::are_we_there_yet ()
{
    return (clean >= TOTAL_PRIMES);
}


// The container's largest prime getter

inline number container::largest_clean ()
{
    return primes[clean--];
}


// The sorter thread's poor sorter

void container::sorter ()
{
    index from, to;
    while (are_we_there_yet ())
    {
        from = clean + 1;
        to = used--;

        trace (("Sorting indexes from: %d to: %d\n", from, to));

        if (from == to)
        {
            trace (("The sorter was bored!\n"));
            std::this_thread::sleep_for
                (std::chrono::milliseconds (BORED_SORTER));

            continue;
        }




        trace (("Done sorting indexes from: %d to: %d\n", from, to));
    }
}


// The container's constructor

container::container ():
    sorter_thread (&container::sorter, this)
{
    // The initial prime!
    primes[0] = 3;
    used = clean = 1;
}


// The main main function

int main()
{
    container data;

    worker_thread* workers[THREADS];

    for (int i = 0; i < THREADS; i++)
        workers[i] = new worker_thread (&data);

    for (int i = 0; i < THREADS; i++)
    {
        workers[i]->join();
        delete workers[i];
    }

    return 0;
}
