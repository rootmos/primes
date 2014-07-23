#ifndef blocking_queue_hpp
#define blocking_queue_hpp

#include <queue>
#include <mutex>
#include <condition_variable>


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
        flag.notify_all ();
        mutex.unlock ();
    };

    T pop ()
    {
        std::unique_lock<std::mutex> lock (mutex);

        flag.wait (lock, [this] { return !queue.empty (); });

        T t = queue.front ();
        queue.pop ();
        return t;
    };

    bool empty ()
    {
        return queue.empty ();
    };
};

#endif
