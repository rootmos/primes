#ifndef PRIMES_HPP
#define PRIMES_HPP


// Settings

#define THREADS 2
#define TOTAL_PRIMES 100
#define BUFFER_SIZE 120
#define CHECK_PRIMES 10     //Roughly: sqrt of TOTAL_PRIMES
#define BORED_SORTER 200
#define ASSIGNMENT_MAX 10

// Includes

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

// Type of the numbers and indexes, unnecessary?

using number = unsigned int;

using index = unsigned int;
using atomic_index = std::atomic<index>;

// Forward decl, move master_info to own set of files?

class master_info;

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

    std::thread sorter_thread;
    void sorter ();

public:

    container();

    bool next_assignment (number& first, number& end);

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


    // Connection to the data

    container* data;

    // The thread
    std::thread thread;

    // The poor method who actually has to work!

    void worker();

public:


    worker_thread (container* data);

    void join ();

};


#endif
