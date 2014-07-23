#ifndef chunk_queue_hpp
#define chunk_queue_hpp

#include <map>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include "debug.hpp"

using uint = unsigned int;


template<typename T>
struct offset_chunk
{
    T* data;
    uint offset;
    uint length;
    uint next_offset;

    offset_chunk ():
        data (nullptr),
        offset (0),
        length (0),
        next_offset (0)
    {};

    offset_chunk (T* d, uint o, uint l, uint n = 0):
        data (d),
        offset (o),
        length (l),
        next_offset (n == 0 ? 2*(l+1) + o : n)
    {};

    bool operator<(const offset_chunk& rhs) const
    {
        return (offset < rhs.offset);
    };
};

template<typename T>
class offset_chunk_queue
{

    std::map<uint, offset_chunk<T> > map;

    std::mutex mutex;
    std::condition_variable flag;

    std::atomic_bool running;

    uint next = 3;

public:

    offset_chunk_queue ():
        running (true)
    {};

    void push (offset_chunk<T> c);
    void push (T* data, uint offset, uint length);

    bool pop (offset_chunk<T>& c);

    bool empty ();

    void stop ();
};

template<typename T>
void offset_chunk_queue<T>::push (offset_chunk<T> c)
{
    mutex.lock ();
    map.insert (std::make_pair(c.offset, c));
    mutex.unlock ();
    flag.notify_one ();
}

template<typename T>
void offset_chunk_queue<T>::push (T* data, uint offset, uint length)
{
    offset_chunk<T> c;
    c.data = data;
    c.offset = offset;
    c.length = length;

    push (c);
}

template<typename T>
bool offset_chunk_queue<T>::pop (offset_chunk<T>& c)
{
    std::unique_lock<std::mutex> lock (mutex);

    flag.wait (lock, [this] { return (!running && map.empty ()) || (map.count(next) != 0); });

    if (!running && map.empty())
        return false;

    auto search = map.find (next);
    c = search->second;
    map.erase (next);
    next = c.next_offset;

    return true;
}

template<typename T>
inline bool offset_chunk_queue<T>::empty ()
{
    return map.empty ();
}

template<typename T>
void offset_chunk_queue<T>::stop ()
{
    std::unique_lock<std::mutex> lock (mutex);

    running = false;
    flag.notify_all();
}

#endif
