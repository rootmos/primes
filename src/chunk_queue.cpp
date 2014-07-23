#include "chunk_queue.hpp"

void chunk_queue::push (chunk c)
{
    mutex.lock ();
    map.insert (std::make_pair(c.offset, c));
    trace (("Pushed chunk with offset=%d.", c.offset));
    flag.notify_all ();
    mutex.unlock ();
}

void chunk_queue::push (bool* data, uint offset, uint length)
{
    chunk c;
    c.data = data;
    c.offset = offset;
    c.length = length;

    push (c);
}

chunk chunk_queue::pop ()
{
    std::unique_lock<std::mutex> lock (mutex);

    trace (("Trying to get chunk which starts with %d.", next));

    flag.wait (lock, [this] { return (map.count(next) != 0); });

    auto search = map.find (next);
    const chunk& c = search->second;
    map.erase (next);
    next = c.offset + 2*c.length;

    trace (("Next chunk to be split starts with %d.", next));

    return c;
}
