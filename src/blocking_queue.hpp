#ifndef blocking_queue_hpp
#define blocking_queue_hpp

#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>

template<typename T>
class blocking_queue
{
    std::queue<T> queue;

    std::mutex mutex;
    std::condition_variable flag;

    std::atomic_bool running;

public:

    blocking_queue ():
        running (true)
    {};

    void push (T t)
    {
        std::unique_lock<std::mutex> lock (mutex);

        queue.push (t);
        flag.notify_one ();
    };

    bool pop (T& t)
    {
        std::unique_lock<std::mutex> lock (mutex);

        flag.wait (lock, [this] { return !running || !queue.empty (); });

        if (!running)
            return false;

        t = queue.front ();
        queue.pop ();
        return true;
    };

    bool empty ()
    {
        return queue.empty ();
    };
    
    void stop ()
    {
        std::unique_lock<std::mutex> lock (mutex);
        running = false;
        
        flag.notify_all ();
    };
};

#endif
