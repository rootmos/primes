#include <iostream>
#include <thread>
#include <cassert>
#include <cmath>
#include <chrono>
#include <mutex>
#include <cstring>
#include <time.h>
#include <queue>
#include <condition_variable>

// For traces and cheat
#include <stdio.h>
#include "config.h"

using uint = unsigned int;

#ifndef NTRACES
#define trace(vars) printf vars
#else
#define trace(vars)
#endif

#ifndef NTESTING
// Read the cheat-file for comparison
uint cheat[TOTAL_PRIMES] = { 0 };

void read_cheat ()
{
    FILE* file = fopen ("cheat", "r");

    assert (file != nullptr);

    for (uint i = 0; i < TOTAL_PRIMES+1; i++)
        assert ( fscanf (file, "%d", &cheat[i]) == 1 );

    fclose (file);
}

bool test (uint i, uint prime)
{
    if ( cheat[i] == 0 )
        read_cheat ();

    return prime == cheat[i];
}

#else

#define test (i, prime)

#endif

class time_block
{
    static std::mutex cerr_lock;
    const clockid_t clock = CLOCK_PROCESS_CPUTIME_ID;
    const long res = 100000;
    timespec start;
    const char* message;
public:

    time_block (const char* m) : message (m)
    {
        clock_gettime (clock, &start);
    };

    ~time_block ()
    {
        timespec end;
        clock_gettime (clock, &end);
        long ns_elapsed = (end.tv_sec - start.tv_sec)*1000000000
            + (end.tv_nsec - start.tv_nsec);
        cerr_lock.lock ();
        std::cerr << message << ": " << ns_elapsed/res << std::endl;
        cerr_lock.unlock ();
    };
};

std::mutex time_block::cerr_lock;
#define time_function() time_block t(__FUNCTION__);

// sqrt (32452843) = 5696 > 5695 = 3+2*2846
#define buffer_length 2847

// 751th prime = 5701, need only 749 since we skip the two
#define factor_length 749

uint factors[factor_length];

#define chunk_length 2000000

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

        trace (("Found prime %d at %d.\n", 3+2*i, i));
    }
}

inline void fill_offset (bool* chunk, uint p, uint offset, uint length)
{
    assert (offset % 2 == 1);
    uint i;
    if ( offset % p == 0)
        i = 0;
    else
    {
        i = offset / p + 1; //ceil (k);

        if ( i % 2 == 0 )
            i += 1;

        i = i*p - offset;

        i /= 2;
    }

    //trace (("Filling i=%d for p=%d with offset=%d.\n", i, p, offset));


    while (i < length)
    {
        chunk[i] = true;
        i += p;
    }
}

static std::mutex output_lock;

inline void output (bool* odds, uint offset, uint length)
{
    return;
    output_lock.lock();
    for (uint i = 0; i < length; i++)
    {
        if (!odds[i])
           std::cout << offset + 2*i << std::endl;
    }
    output_lock.unlock();
}

struct chunk
{
    bool* data;
    uint offset;
    uint length;

    inline bool operator<(const chunk& rhs)
    {
        return (offset < rhs.offset);
    };
};



class chunk_queue
{
    std::priority_queue<chunk> queue;

    std::mutex mutex;
    std::condition_variable flag;

    uint next = 3;

public:

    void push (chunk c)
    {
        mutex.lock ();
        queue.push (c);
        mutex.unlock ();

        flag.notify_all ();
    };

    chunk pop ()
    {
        std::unique_lock<std::mutex> lock (mutex);

        flag.wait (lock, [this] { return (queue.top().offset == next); });

        const chunk& c = queue.top ();
        queue.pop ();
        next = c.offset + c.length + 2;

        return c;
    };

};

struct output_chunk
{
    char *buffer;
    int length;
};

template<typename T>
class blocking_queue
{
    std::queue<T> queue;

    std::mutex mutex;
    std::condition_variable flag;

public:

    void push (T t)
    {
        mutex.lock ();
        queue.push (t);
        mutex.unlock ();

        flag.notify_all ();
    };

    T pop ()
    {
        std::unique_lock<std::mutex> lock (mutex);

        flag.wait (lock, [this] { return !queue.empty (); });

        T t = queue.top ();
        queue.pop ();
        return t;
    };
};


blocking_queue<output_chunk> output_queue;

void prepare_chunk (const chunk& c, uint*& primes, uint& number)
{
    //TODO: can we make this smaller? E.g. by estimating using the factors?
    primes = new uint[c.length];

    number = 0;
    bool* itr = c.data;
    bool* end = c.data + c.length;
    while ( itr < end )
    {
        if (!*itr)
            primes [number++] = itr - c.data + c.offset;

        itr++;
    }
}



void offset_sieve (bool* chunk, uint from, uint to)
{
    time_function ();
    uint length = to - from;
    length = (length > 2*chunk_length ? chunk_length : length/2);

    trace (("Sieving from %d to %d.\n", from, from+2*length));

    for (uint i = 0; i < factor_length; i++)
    {
        fill_offset (chunk, factors[i], from, length);
    }

    output (chunk, from, length);

    std::this_thread::yield ();

    if ( to - from  > 2*chunk_length )
    {
        for (uint j = 0; j < chunk_length; j++ )
            chunk[j] = false;
        //memset (chunk, 0, chunk_length);
        offset_sieve (chunk, from + 2*length, to);
    }
}

void worker (uint from, uint to)
{
    trace (("I was assigned %d to %d.\n", from, to));
    bool* chunk = new bool[chunk_length];

    offset_sieve (chunk, from, to);
    trace (("I finished %d to %d.\n", from, to));
}

// The main main function

int main()
{
    // TODO: output 2 somewhere more convenient, i.e. when we open the file for
    // writing?
    // std::cout << 2 << std::endl;


    time_function ();

    bool odds[buffer_length] = { false };
    sieve (odds);

    // Output the first factors we've found

    output (odds, 3, factor_length);

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

    std::thread* workers[THREADS];

    uint from = factors[i-1]+2;
    uint to = 32452845;
    uint end;
    uint assignment_length = (to - from)/THREADS + 1;

    for (int j = 0; j < THREADS; j++)
    {
        if (j+1 == THREADS)
            end = to;
        else
            end = from + assignment_length;

        workers[j] = new std::thread (&worker, from, end);
        from += assignment_length;
    }

    for (int j = 0; j < THREADS; j++)
        workers[j]->join();

    return 0;
}


