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

worker_thread::worker_thread (container* d, worker_thread* sibling):
    next_sibling(nullptr),
    previous_sibling(sibling),
    data(d),
    thread(&worker_thread::worker, this)
{
    if (previous_sibling != nullptr)
        previous_sibling->next_sibling = this;
}

// The worker thread's join method

worker_thread::~worker_thread ()
{
    thread.join();
}


number worker_thread::update_lowest_assignment()
{
    worker_thread* itr = previous_sibling;

    while (itr != nullptr)
    {
        if (itr->assignment_end != 0 &&
            itr->assignment_end < assignment_end)
            return 0;

        itr = itr->previous_sibling;        
    }

    itr = next_sibling;
    while (itr != nullptr)
    {
        if (itr->assignment_end != 0 &&
            itr->assignment_end < assignment_end)
            return 0;

        itr = itr->next_sibling;        
    }

    return assignment_end;
}


// The worker thread's actual worker!

void worker_thread::worker ()
{
    container::iterator itr = data->get_iterator();
    number divisor, divisor_limit, remainder;

    while ( data->next_assignment (this, current, assignment_end) )
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
    trace (("Found prime! %d: %d\n", (index)used, prime));
    std::lock_guard<std::mutex> lock (report_prime_mutex);

    primes[used] = prime;

    used++;

    assert (used < BUFFER_SIZE); // What should we do when this fails?
}


// The container's next_assignment

bool container::next_assignment (worker_thread* thread,
                                 number& first, number& end)
{
    std::unique_lock<std::mutex> lock(assignment_mutex);

    if (are_we_there_yet())
        return false;
    
    // Update the linked list of workers and the lowes_assigned
    // We do this here since we may need to use the value of end 

    number update = thread->update_lowest_assignment();

    if (update)
    {
        lowest_completed = update;
        trace (("The new lowest_completed is %d\n", (number)lowest_completed)); 
    }

    // Perhaps we need to wait for new clean, fresh, primes
    new_clean_primes.wait
        (lock,
         [this]
            { return std::pow(largest_clean(),2) > (largest_assigned + 2);} );

    // Calculate the first number of the new assignment
    first = largest_assigned + 2;
    assert ( first % 2 == 1 ); // first should be an odd number
  
    // Calculate the end of the assignment
    end = first + (std::pow(largest_clean(),2) - first) / THREADS;

    if (end - first > ASSIGNMENT_MAX) // We should not take to large bites
        end = first + ASSIGNMENT_MAX;
    else if (end % 2 == 0) // and we want to end at an odd number
        end--;

    assert (first <= end);

    // Update the largest handed out
    largest_assigned = end;

    return true;
}


// The container's nagging child

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


number* partition (number* pivot, number* start, number* end)
{
    std::swap(*pivot, *end);

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

    return i;
}


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

    number* i = partition (&start[pivot_index], start, end);

    // Sort the two partitions
    if (start < i - 1 )
        quicksort(start, i-1);
    if ( i+1 <= end )
        quicksort(i+1, end);
}

index quicksort_find_pivot_and_skip_top (number pivot, number* start, number* end)
{
    assert (start <= end);
    if (start == end) // A list of length 1 is always sorted
        return 1;
    else if (end == start + 1) // A list of length 2 is easy to sort
    {
        if ( *start > *end )
            std::swap(*start, *end);

        if ( *start == pivot )
            return 1;
        if ( *end == pivot )
            return 2;
        else
        {
            assert ( 0 && "Couldn't find pivot in sort" );
            return 0;
        }
    }

    number* i = start;
    while (i <= end)
    {
        if ( *i == pivot )
            break;
        else
            i++;
    }
    
    if ( i > end )
    {
        assert ( 0 && "Couldn't find pivot in sort" );
        return 0;
    }

    i = partition (i, start, end);

    // Sort only the bottom partition
    if (start < i - 1 )
        quicksort(start, i-1);

    return (i - start);
}



// The sorter thread's poor sorter

void container::sorter ()
{
    std::unique_lock<std::mutex> lock(assignment_mutex, std::defer_lock);
    index from, to;
    while (!are_we_there_yet ())
    {
        from = clean;
        to = used - 1;

        number pivot = lowest_completed;

        if (from > to || pivot <= primes[clean-1])
        {
            trace (("The sorter was bored!\n"));
            std::this_thread::sleep_for
                (std::chrono::milliseconds (BORED_SORTER));

            continue;
        }

        trace (("Sorting indexes from: %d to: %d. With pivot %d\n",
                from, to, pivot));


        index new_clean = 
            quicksort_find_pivot_and_skip_top (pivot,
                                               &primes[from],
                                               &primes[to]);

        trace (("Done sorting indexes from: %d to: %d\n", from, to));

        // Update the number of clean primes and signal waiting threads

        lock.lock();

        clean += new_clean;

        new_clean_primes.notify_all();
        lock.unlock();

        // Output the sorted primes

        for (index i = from; i < clean && i <= TOTAL_PRIMES; i++)
            std::cout << primes[i] << std::endl;

    }
}


// The container's constructor

container::container ():
    head(nullptr),
    tail(nullptr),
    sorter_thread (&container::sorter, this)
{
    // The initial prime!
    primes[0] = largest_assigned = 3;
    lowest_completed = 3;
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
        workers[i] = new worker_thread (&data,
                                        i > 0? workers[i-1] : nullptr);

    for (int i = 0; i < THREADS; i++)
        delete workers[i];

    return 0;
}


