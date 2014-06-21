#include <iostream>
#include <thread>
#include <cassert>
#include <cmath>
#include <chrono>

#include "primes.hpp"
#include "config.h"


#ifndef NTRACES
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
    // We should be able to stay within our thead's cache
    // Perhaps: make this array grow dynamically?
    assert ( i < CHECK_PRIMES );

    if (i < stored)
    {
        return primes[i++];
    }

    // We should only need to fetch the next prime
    assert ( i == stored );

    primes[i] = data->get (i);
    
    stored++;

    return primes[i++];
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
    parent(nullptr),
    child(nullptr),
    data(d),
    thread(&worker_thread::worker, this)
{
}

// The worker thread's join method

worker_thread::~worker_thread ()
{
    thread.join();
}

// The worker thread's move_to_end method

worker_thread* worker_thread::move_to_end(worker_thread* new_parent)
{
    if (child == nullptr) // This means that we already are the last
    {
        assert (new_parent == this);
        return nullptr;
    }

    assert (new_parent != nullptr); // We don't like to be passed nullptr!
    assert (new_parent->child == nullptr); // We should be added at the end

    new_parent->child = this;
    
    if (parent != nullptr)
    {
        // If we have a parent we just remove ourselves from the chain
        parent->child = child;
        child = nullptr;
        return nullptr;
    }
    else
    {
        // We are the first thread, so we return our child which becomes
        // the new first thread
        
        worker_thread* retptr = child;
        child = nullptr;
        return retptr;
    }
}


// The worker thread's actuall worker!

void worker_thread::worker ()
{
    container::iterator itr = data->get_iterator();
    number divisor, divisor_limit, remainder;

    while ( data->next_assignment (current, assignment_end) )
    {
        trace (("%ld: I was assigned: %d to %d\n",
                (uintptr_t)this, current, assignment_end));

        // Ensure that we actually start with an odd number
        assert( current % 2 == 1 );

        while ( current <= assignment_end )
        {
            itr.reset();
            divisor_limit = std::sqrt (current);
            remainder = 1;
            
            //trace (("%ld: Testing: %d with divisor limit: %d\n",
            //        (uintptr_t)this, current, divisor_limit));

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

        //trace (("%ld: I finished checking my interval (up to %d)\n",
        //        (uintptr_t)this, assignment_end));
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
            { return std::pow(largest_clean(),2) > (largest_assigned + 2);} );

    first = largest_assigned + 2;
    assert ( first % 2 == 1 ); // first should be an odd number

    end = first + (std::pow(largest_clean(),2) - first) / THREADS;

    if (end - first > ASSIGNMENT_MAX) // We should not take to large bites
        end = first + ASSIGNMENT_MAX;
    else if (end % 2 == 0) // and we want to end at an odd number
        end--;

    assert (first <= end);

    largest_assigned = end;

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
    return primes[clean - 1];
}




// Quicksort

void quicksort (number* start, number* end)
{
    assert (start <= end);
    if (start == end) // A list of length 1 is always sorted
        return;
    else if (end == start + 1) // A list of length 2 is easy to sort
    {
        if ( *start > *end )
            std::swap(*start, *end);

        return;
    }

    // Let's choose the middle element as pivot
    size_t pivot_index = (end-start)/2;

    std::swap(start[pivot_index], *end);

    number* i = start;
    number* j;

    while (i < end)
    {
        // Find the first value larger than the pivot
        while (i < end && *i <= *end) // Remember: *end is our pivot value
            i++;

        if (i == end)
            break;

        // Let's look for a smaller value to swap with

        j = i + 1;

        while (j < end && *j >= *end)
            j++;

        if (j == end)
        {
            //We couldn't find a smaller pivot, i.e. the parititon is done
            std::swap(*i, *end);
            break;
        }
        else
        {
            // Found a smaller element than pivor, i.e. we swap!
            std::swap(*i, *j);
        }
    }

    // Sort the two partitions
    if (start < i - 1 )
        quicksort(start, i-1);
    if ( i+1 <= end )
        quicksort(i+1, end);
}



// The sorter thread's poor sorter

void container::sorter ()
{
    std::unique_lock<std::mutex> lock(assignment_mutex, std::defer_lock);
    index from, to;
    while (!are_we_there_yet ())
    {
        from = clean + 1;
        to = used - 1;


        if (from > to)
        {
            trace (("The sorter was bored!\n"));
            std::this_thread::sleep_for
                (std::chrono::milliseconds (BORED_SORTER));

            continue;
        }

        if (to - from > 10)
            to = from + 10;

        trace (("Sorting indexes from: %d to: %d\n", from, to));

        quicksort (&primes[from], &primes[to]);

        trace (("Done sorting indexes from: %d to: %d\n", from, to));

        // Update the number of clean primes and signal waiting threads

        lock.lock();

        clean = to;

        new_clean_primes.notify_all();
        lock.unlock();

        // Output the sorted primes

        for (index i = from; i <= to && i <= TOTAL_PRIMES; i++)
            std::cout << primes[i] << std::endl;

    }
}


// The container's constructor

container::container ():
    sorter_thread (&container::sorter, this)
{
    // The initial prime!
    primes[0] = largest_assigned = 3;
    used = clean = 1;
}

// The container's destructor

container::~container()
{
    sorter_thread.join();
}


// The main main function

int main()
{
    container data;

    worker_thread* workers[THREADS];

    for (int i = 0; i < THREADS; i++)
        workers[i] = new worker_thread (&data);

    for (int i = 0; i < THREADS; i++)
        delete workers[i];

    return 0;
}


