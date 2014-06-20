#ifndef PRIMES_HPP
#define PRIMES_HPP


// Settings

#define THREADS 2

#define TOTAL_PRIMES 100
#define CHECK_PRIMES 10     //Roughly: sqrt of TOTAL_PRIMES

// Includes

#include <thread>


// Type of the numbers and indexes, unnecessary?

using number = unsigned int;
using index = unsigned int;


// Forward decl, move master_info to own set of files?

class master_info;

// The data container

class container
{

    number primes[TOTAL_PRIMES];
    index found;
    index clean;

    number get (index i);

public:

    void report_prime (number prime);

    // Forward declaration of the iterator
    class iterator;

    iterator get_iterator();

    class iterator
    {
        number primes[CHECK_PRIMES];
        index i;
        index stored;

        container* data;
        
        friend iterator container::get_iterator ();
        iterator (container* data);

    public:

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

    
    // Connections

    master_info* master;
    container* data;
    worker_thread* next;

    // The thread
    std::thread thread;

    // The poor method who actually has to work!

    void worker();

public:


    worker_thread (master_info* master, container* data);
    
    void set_next_thread (worker_thread* nex);

    void join ();

};



//The master info container/thread controller

class master_info
{
    worker_thread* first;
    worker_thread* last;

    container data;

public:

    master_info ();


    bool next_assignment (number& lower, number& upper);

};




#endif
