#include <iostream>
#include <thread>
#include <cassert>
#include <cmath>

#include "primes.hpp"

// Worker thread's iterator constructor

container::iterator::iterator (container* d):
    i(0),
    stored(0),
    data(d)
{
}


// Worker thread's iterator reset

void container::iterator::reset ()
{
    this->i = 0;
}



// Worker thread's iterator next

number container::iterator::next ()
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

number container::get (index i)
{
    // We should only try to retrieve clean read-only data
    assert(i < clean);

    return primes[i];
}


// The container's get_iterator function

container::iterator container::get_iterator ()
{
    return iterator(this);
}



// The worker thread's constructor

worker_thread::worker_thread (master_info* m, container* d):
    master(m),
    data(d),
    next(nullptr),
    thread(&worker_thread::worker, this)
{
}


// The worker thread's next thread setter

void worker_thread::set_next_thread (worker_thread* n)
{
    next = n;
}

// The worker thread's join method

void worker_thread::join ()
{
    thread.join();
}


// The worker thread's actuall worker!

void worker_thread::worker ()
{
    container::iterator itr = data->get_iterator();
    number divisor, divisor_limit, remainder;

    while ( master->next_assignment (current, assignment_end) )
    {
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
    }

}


// The container's report_prime method

void container::report_prime (number prime)
{
    std::lock_guard<std::mutex> lock (report_prime_mutex);

    primes[used] = prime;

    used++;

    assert (used < BUFFER_SIZE); // What should we do when this fails?
}


// The master_info's next_assignment

bool master_info::next_assignment (number& first, number& end)
{
    std::unique_lock<std::mutex> lock(assignment_mutex);

    if (data.are_we_there_yet())
        return false;

    new_clean_primes.wait
        (lock,
         [this]
            { return std::pow(data.largest_clean(),2) < largest_assigned;} );

    first = largest_assigned + 2;
    assert ( first % 2 == 1 ); // first should be an odd number

    largest_assigned = end = std::pow(data.largest_clean(),2) / THREADS;
    if (end % 2 == 1)
        end--;

    assert (first < end); // Could the opposite really occur?

    return true;
}


// The container's are_we_there_yet method

bool container::are_we_there_yet()
{
    return (clean >= TOTAL_PRIMES);
}


// The container's largest prime getter

number container::largest_clean ()
{
    return primes[clean--];
}


// The main main function

int main()
{

    return 0;
}
