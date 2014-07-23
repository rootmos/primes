#ifndef chunk_queue_hpp
#define chunk_queue_hpp

#include <map>
#include <mutex>
#include <condition_variable>

#include "debug.hpp"

using uint = unsigned int;


struct chunk
{
    bool* data;
    uint offset;
    uint length;

    bool operator<(const chunk& rhs) const
    {
        return (offset < rhs.offset);
    };
};


class chunk_queue
{

    std::map<uint, chunk> map;

    std::mutex mutex;
    std::condition_variable flag;

    uint next = 3;

public:

    void push (chunk c);
    void push (bool* data, uint offset, uint length);

    chunk pop ();
};

#endif
