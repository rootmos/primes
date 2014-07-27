#ifndef blocking_queue_hpp
#define blocking_queue_hpp

#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>

template<typename T>
class blocking_queue
{
    std::queue<T> queue;

    std::mutex mutex;
    std::condition_variable flag;

    std::atomic_bool running;

    bool pop_or_not ()
    {
        return !running || !queue.empty();
    };
    
    typedef std::function<bool (const T&)> Predicate;

    bool pop_or_not_with_predicate (Predicate &p)
    {
        return !running || ( !queue.empty() && p(queue.front ()) );
    };


public:

    blocking_queue ():
        running (true)
    {};

    void push (const T& t)
    {
        std::unique_lock<std::mutex> lock (mutex);

        queue.push (t);
        flag.notify_one ();
    };

    bool pop (T& t)
    {
        std::unique_lock<std::mutex> lock (mutex);

        flag.wait (lock, std::bind (&blocking_queue::pop_or_not, this)); //[this] { return !running || !queue.empty (); });

        if (!running)
            return false;

        t = queue.front ();
        queue.pop ();
        return true;
    };


    bool pop_if (T& t, Predicate& p)
    {
        std::unique_lock<std::mutex> lock (mutex);

        flag.wait (lock, std::bind (&blocking_queue::pop_or_not_with_predicate,
                                    this, p));

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
