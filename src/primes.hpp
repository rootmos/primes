#ifndef PRIMES_HPP
#define PRIMES_HPP

// Includes

#include "config.h"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

// Type of the numbers and indexes, unnecessary?

using number = unsigned int;

using index = unsigned int;
using atomic_index = std::atomic<index>;

// Forward decl, move worker_thread to own set of files?

class worker_thread;

// The data container

class container
{
    number largest_assigned;
    std::mutex assignment_mutex;
    std::condition_variable new_clean_primes;


    std::mutex report_prime_mutex;

    number primes[BUFFER_SIZE];
    atomic_index used;
    
    // Number of sorted primes, i.e. not a zero-based index
    atomic_index clean;

    number get (index i);

    // The start of the linked list of worker_threads
    worker_thread* head;
    worker_thread* tail;
    atomic_index lowest_assigned;

    // The sorter thread's thread and worker

    std::thread sorter_thread;
    void sorter ();

public:

    container();
    ~container();

    bool next_assignment (worker_thread* thread,
                          number& first,
                          number& end);

    void report_prime (number prime);

    bool are_we_there_yet();

    number largest_clean();


    // Forward declaration of the iterator
    class iterator;

    iterator get_iterator();

    class iterator
    {
        // The thread-local cache, i.e. we hope that this access is quicker
        number primes[CHECK_PRIMES];
        index i;
        index stored;

        container* data;

        friend iterator container::get_iterator ();
        iterator (container* data);

    public:

        // TODO: Move and copy constructors!

        number get (index i); // References or pointers for performance?
        number next ();

        void reset ();
    };


};


// The worker thread

class worker_thread
{

    // Current assignment

    number current;
    number assignment_end;

    
    // The therad's parent and child
    
    worker_thread* parent;
    worker_thread* child;


    // Connection to the data

    container* data;

    // The thread
    std::thread thread;

    // The poor method who actually has to work!

    void worker();

public:

    worker_thread* move_to_end(worker_thread* new_parent);

    worker_thread (container* data);
    ~worker_thread();

};


#endif
